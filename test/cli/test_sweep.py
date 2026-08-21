#!/usr/bin/env python3
"""btk sweep CLI tests. Offline mock RPC only."""

import json
import os
import pathlib
import socket
import subprocess
import sys
import tempfile
import threading

ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
BTK = ROOT / "bin" / "btk"

SWEEP_HELP = """btk sweep — spend confirmed coins to one address

Usage:
  btk sweep --to ADDRESS
            [--fee-rate SATVB] [--dry-run]
            [--host H] [--port P] [--rpc-auth USER:PASS]
            [--type p2pkh|p2wpkh|p2tr]... [--source] [--verbose]

Sweep every confirmed UTXO at a spendable address to --to.
Looks up coins with Core scantxoutset (the node's UTXO set, not
~/.btk/balance). Signs in process. Broadcasts with
sendrawtransaction. Core does not need a wallet. Mainnet only.

Input is stdin only (no positional keys). Items are a typed address
object whose source contains a privkey, a typed privkey object, or
a bare WIF. Nested source is walked (up to 8 levels). The from
address is re-derived from the secret; a mismatch is "private key
does not match address". A typed privkey (or bare WIF) defaults to
one p2wpkh; --type is repeatable and selects which of that key's
addresses to sweep. p2sh and p2wsh cannot be spent.
64-hex, leftover text, a pubkey without a secret, and an address
without a privkey source are errors.

--to is required and may be p2pkh, p2sh, p2wpkh, p2wsh, or p2tr.
All mature UTXOs become inputs; one output is --to minus fee.
Fee is --fee-rate sat/vB, or estimatesmartfee 6. Dummy-signature
vsize is used so extra sats go to fee, not change. nLockTime is
the scan height; nSequence is 0xfffffffd (RBF). Immature coinbase
is skipped. Empty stdin is empty stdout, exit 0.

No cookie file. Use --rpc-auth user:pass or config rpc.auth.
--host / --port default to config rpc.host / rpc.port or
127.0.0.1 / 8332. A UTXO-set scan can take minutes.

Options:
  -h, --help             Show this help and exit
      --to ADDRESS       Destination mainnet address. Required.
      --fee-rate SATVB   Fee rate in sat/vB. Default: Core
                         estimatesmartfee 6
      --dry-run          Sign and print; do not broadcast
      --host HOST        RPC host. Default: config rpc.host or
                         127.0.0.1. A host:port form sets the port.
      --port PORT        RPC port. Default: config rpc.port or 8332
      --rpc-auth USER:PASS
                         HTTP Basic credentials. Default: config
                         rpc.auth. No cookie file.
      --type TYPE        With a privkey input, which address to
                         sweep. Repeatable. Default: p2wpkh
      --source           Include the input item as a source object
      --verbose          Include the raw transaction hex
      --config PATH      Config file. Default: $BTK_CONFIG, else
                         ~/.btk/config.json
  -o, --out FORMAT       ndjson (default), json, or plain. plain
                         prints the txid.
      --in FORMAT        auto (default), ndjson, json, or plain

Examples:
  btk privkey --new | btk address --source | btk sweep --to bc1q...
  printf '%s' KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn \\
    | btk sweep --to 1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH --dry-run
"""

WIF_G = "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn"
P2WPKH_G = "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4"
P2PKH_G = "1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH"
P2TR_G = "bc1pmfr3p9j00pfxjh0zmgp99y8zftmd3s5pmedqhyptwy6lm87hf5sspknck9"
DEST = "bc1q7499s50fxu4c0qg23esvm5h8elvqkm33r2tdza"
SPK_P2WPKH_G = "0014751e76e8199196d454941c45d1b3a323f1433bd6"
TXID_DISPLAY = "a882c290a9ce09514de2f57236e349d76b6e4f8437d16d2e132ffc2dada2583c"


def start_rpc(unspents, height=800000, feerate=0.0001, send_txid="00" * 32, fail_send=False):
    """unspents: list of dicts as Core scantxoutset entries. feerate is BTC/kvB."""
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind(("127.0.0.1", 0))
    srv.listen(16)
    srv.settimeout(0.2)
    port = srv.getsockname()[1]
    stop = threading.Event()
    errors = []
    calls = []

    def handle_one(conn):
        conn.settimeout(5)
        data = b""
        while b"\r\n\r\n" not in data:
            chunk = conn.recv(4096)
            if not chunk:
                return
            data += chunk
        head, rest = data.split(b"\r\n\r\n", 1)
        clen = 0
        for line in head.decode("latin1").split("\r\n"):
            if line.lower().startswith("content-length:"):
                clen = int(line.split(":", 1)[1].strip())
        while len(rest) < clen:
            rest += conn.recv(clen - len(rest))
        req = json.loads(rest.decode())
        method = req.get("method")
        params = req.get("params") or []
        calls.append({"method": method, "params": params})
        result = None
        error = None
        if method == "scantxoutset":
            result = {
                "success": True,
                "txouts": 1,
                "height": height,
                "bestblock": "00" * 32,
                "unspents": unspents,
                "total_amount": 0,
            }
        elif method == "estimatesmartfee":
            if feerate is None:
                result = {"errors": ["Insufficient data"]}
            else:
                result = {"feerate": feerate, "blocks": 6}
        elif method == "sendrawtransaction":
            if fail_send:
                error = {"code": -26, "message": "min relay fee not met"}
            else:
                result = send_txid
        else:
            raise RuntimeError("unknown method " + str(method))
        body = json.dumps({"result": result, "error": error, "id": req.get("id")}).encode()
        resp = (
            b"HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nContent-Length: "
            + str(len(body)).encode()
            + b"\r\n\r\n"
            + body
        )
        conn.sendall(resp)

    def run_loop():
        try:
            while not stop.is_set():
                try:
                    conn, _ = srv.accept()
                except socket.timeout:
                    continue
                with conn:
                    try:
                        handle_one(conn)
                    except Exception as exc:  # pragma: no cover
                        errors.append(exc)
        finally:
            srv.close()

    t = threading.Thread(target=run_loop, daemon=True)
    t.start()
    return port, stop, t, errors, calls


def run(args, input_bytes=None, home=None, timeout=10):
    if isinstance(input_bytes, str):
        input_bytes = input_bytes.encode()
    env = os.environ.copy()
    if home is not None:
        env["HOME"] = home
    return subprocess.run(
        [str(BTK), *args],
        input=input_bytes,
        capture_output=True,
        timeout=timeout,
        cwd=str(ROOT),
        env=env,
    )


def ndjson(stdout):
    return [json.loads(ln) for ln in stdout.decode().splitlines() if ln]


def p2wpkh_utxo(amount=0.001, coinbase=False, utxo_height=700000):
    return {
        "txid": TXID_DISPLAY,
        "vout": 0,
        "scriptPubKey": SPK_P2WPKH_G,
        "amount": amount,
        "coinbase": coinbase,
        "height": utxo_height,
    }


def main():
    r = run(["sweep", "--help"])
    assert r.returncode == 0, r.stderr.decode()
    assert r.stdout.decode() == SWEEP_HELP, repr(r.stdout.decode())

    with tempfile.TemporaryDirectory() as home:
        r = run(["sweep"], input_bytes=WIF_G + "\n", home=home)
        assert r.returncode == 1
        assert "missing destination" in r.stderr.decode()

        r = run(["sweep", "--to", DEST, "leftover"], input_bytes=WIF_G + "\n", home=home)
        assert r.returncode == 1
        assert "provide input on stdin" in r.stderr.decode()

        r = run(["sweep", "--to", DEST, "--stream"], input_bytes=b"", home=home)
        assert r.returncode == 1
        assert "sweep does not stream" in r.stderr.decode()

        r = run(["sweep", "--to", DEST, "--count", "1"], input_bytes=b"", home=home)
        assert r.returncode == 1
        assert "unknown option '--count'" in r.stderr.decode()

        r = run(["sweep", "--to", DEST, "--from", "wif"], input_bytes=b"", home=home)
        assert r.returncode == 1
        assert "unknown option '--from'" in r.stderr.decode()

        r = run(["sweep", "--to", "tb1qw508d6qejxtdg4y5r3zarvary0c5xw7kxpjzsx"], home=home)
        assert r.returncode == 1
        assert "not a bitcoin address" in r.stderr.decode()

        r = run(["sweep", "--to", DEST], input_bytes=b"", home=home)
        assert r.returncode == 0, r.stderr.decode()
        assert r.stdout == b""

        r = run(["sweep", "--to", DEST], input_bytes="not-a-key\n", home=home)
        assert r.returncode == 1
        assert "not a private key" in r.stderr.decode()

        r = run(["sweep", "--to", DEST], input_bytes=P2WPKH_G + "\n", home=home)
        assert r.returncode == 1
        assert "missing private key" in r.stderr.decode()

        r = run(
            ["sweep", "--to", DEST],
            input_bytes=json.dumps({"type": "address", "style": "p2wpkh", "data": P2WPKH_G}) + "\n",
            home=home,
        )
        assert r.returncode == 1
        assert "missing private key" in r.stderr.decode()

        r = run(
            ["sweep", "--to", DEST],
            input_bytes=json.dumps(
                {
                    "type": "address",
                    "style": "p2wpkh",
                    "data": DEST,
                    "source": {
                        "type": "privkey",
                        "encoding": "wif",
                        "data": WIF_G,
                        "compressed": True,
                        "network": "mainnet",
                    },
                }
            )
            + "\n",
            home=home,
        )
        assert r.returncode == 1
        assert "private key does not match address" in r.stderr.decode()

        r = run(
            ["sweep", "--to", DEST, "--type", "p2sh"],
            input_bytes=WIF_G + "\n",
            home=home,
        )
        assert r.returncode == 1
        assert "cannot spend p2sh" in r.stderr.decode()

        port, stop, thread, errors, calls = start_rpc([])
        r = run(
            [
                "sweep",
                "--to",
                DEST,
                "--host",
                "127.0.0.1",
                "--port",
                str(port),
                "--fee-rate",
                "10",
                "--dry-run",
            ],
            input_bytes=WIF_G + "\n",
            home=home,
        )
        stop.set()
        thread.join(timeout=5)
        assert r.returncode == 1, (r.stderr.decode(), r.stdout.decode(), errors)
        assert "no unspent outputs" in r.stderr.decode()
        assert "scanning utxo set" in r.stderr.decode()
        assert any(c["method"] == "scantxoutset" for c in calls)
        assert not any(c["method"] == "sendrawtransaction" for c in calls)

        port, stop, thread, errors, calls = start_rpc(
            [p2wpkh_utxo(coinbase=True, utxo_height=799950)]
        )
        r = run(
            [
                "sweep",
                "--to",
                DEST,
                "--host",
                "127.0.0.1",
                "--port",
                str(port),
                "--fee-rate",
                "10",
                "--dry-run",
            ],
            input_bytes=WIF_G + "\n",
            home=home,
        )
        stop.set()
        thread.join(timeout=5)
        assert r.returncode == 1, r.stderr.decode()
        assert "no unspent outputs" in r.stderr.decode()

        port, stop, thread, errors, calls = start_rpc([p2wpkh_utxo()])
        r = run(
            [
                "sweep",
                "--to",
                DEST,
                "--host",
                "127.0.0.1",
                "--port",
                str(port),
                "--fee-rate",
                "10",
                "--dry-run",
            ],
            input_bytes=WIF_G + "\n",
            home=home,
        )
        stop.set()
        thread.join(timeout=5)
        assert r.returncode == 0, (r.stderr.decode(), r.stdout.decode(), errors)
        obj = ndjson(r.stdout)[0]
        assert obj["type"] == "tx"
        assert obj["from"] == P2WPKH_G
        assert obj["to"] == DEST
        assert obj["broadcast"] is False
        assert obj["inputs"] == 1
        assert obj["fee"] + obj["sats"] == 100000
        assert obj["sats"] > 0
        assert "hex" in obj
        assert not any(c["method"] == "sendrawtransaction" for c in calls)
        desc = calls[0]["params"][1][0]
        assert desc == "addr(" + P2WPKH_G + ")"

        port, stop, thread, errors, calls = start_rpc([p2wpkh_utxo()], send_txid="ab" * 32)
        r = run(
            [
                "sweep",
                "--to",
                DEST,
                "--host",
                "127.0.0.1",
                "--port",
                str(port),
                "--fee-rate",
                "10",
                "--verbose",
            ],
            input_bytes=WIF_G + "\n",
            home=home,
        )
        stop.set()
        thread.join(timeout=5)
        assert r.returncode == 0, (r.stderr.decode(), r.stdout.decode(), errors)
        obj = ndjson(r.stdout)[0]
        assert obj["broadcast"] is True
        assert "hex" in obj
        sends = [c for c in calls if c["method"] == "sendrawtransaction"]
        assert len(sends) == 1
        assert sends[0]["params"][0] == obj["hex"]

        sourced = json.dumps(
            {
                "type": "address",
                "style": "p2wpkh",
                "network": "mainnet",
                "data": P2WPKH_G,
                "source": {
                    "type": "privkey",
                    "encoding": "wif",
                    "network": "mainnet",
                    "compressed": True,
                    "data": WIF_G,
                },
            }
        )
        port, stop, thread, errors, calls = start_rpc([p2wpkh_utxo()])
        r = run(
            [
                "sweep",
                "--to",
                DEST,
                "--host",
                "127.0.0.1",
                "--port",
                str(port),
                "--fee-rate",
                "10",
                "--dry-run",
                "--source",
                "--out",
                "plain",
            ],
            input_bytes=sourced + "\n",
            home=home,
        )
        stop.set()
        thread.join(timeout=5)
        assert r.returncode == 0, r.stderr.decode()
        txid = r.stdout.decode().strip()
        assert len(txid) == 64

        port, stop, thread, errors, calls = start_rpc([p2wpkh_utxo()])
        r = run(
            [
                "sweep",
                "--to",
                DEST,
                "--host",
                "127.0.0.1",
                "--port",
                str(port),
                "--fee-rate",
                "10",
                "--dry-run",
                "--source",
            ],
            input_bytes=sourced + "\n",
            home=home,
        )
        stop.set()
        thread.join(timeout=5)
        assert r.returncode == 0, r.stderr.decode()
        obj = ndjson(r.stdout)[0]
        assert obj["source"]["type"] == "address"
        assert obj["source"]["source"]["type"] == "privkey"

        port, stop, thread, errors, calls = start_rpc([p2wpkh_utxo()])
        r = run(
            [
                "sweep",
                "--to",
                DEST,
                "--host",
                "127.0.0.1",
                "--port",
                str(port),
                "--fee-rate",
                "10",
                "--dry-run",
                "--source",
            ],
            input_bytes=WIF_G + "\n",
            home=home,
        )
        stop.set()
        thread.join(timeout=5)
        assert r.returncode == 0, r.stderr.decode()
        obj = ndjson(r.stdout)[0]
        assert obj["source"]["type"] == "privkey"
        assert obj["source"]["encoding"] == "wif"
        assert obj["source"]["data"] == WIF_G
        assert "_bare" not in obj["source"]

        port, stop, thread, errors, calls = start_rpc([p2wpkh_utxo()], feerate=None)
        r = run(
            [
                "sweep",
                "--to",
                DEST,
                "--host",
                "127.0.0.1",
                "--port",
                str(port),
                "--dry-run",
            ],
            input_bytes=WIF_G + "\n",
            home=home,
        )
        stop.set()
        thread.join(timeout=5)
        assert r.returncode == 1
        assert "fee rate unavailable (pass --fee-rate)" in r.stderr.decode()

        port, stop, thread, errors, calls = start_rpc([p2wpkh_utxo()], feerate=0.0001)
        r = run(
            [
                "sweep",
                "--to",
                DEST,
                "--host",
                "127.0.0.1",
                "--port",
                str(port),
                "--dry-run",
            ],
            input_bytes=WIF_G + "\n",
            home=home,
        )
        stop.set()
        thread.join(timeout=5)
        assert r.returncode == 0, r.stderr.decode()
        assert any(c["method"] == "estimatesmartfee" for c in calls)
        obj = ndjson(r.stdout)[0]
        # 0.0001 BTC/kvB = 10 sat/vB
        assert obj["fee"] % 10 == 0

        r = run(
            ["sweep", "--to", DEST, "--fee-rate", "0"],
            input_bytes=WIF_G + "\n",
            home=home,
        )
        assert r.returncode == 1
        assert "invalid --fee-rate" in r.stderr.decode()

        # Dust: 400 sats cannot cover a P2WPKH sweep at 10 sat/vB
        port, stop, thread, errors, calls = start_rpc([p2wpkh_utxo(amount=0.00000400)])
        r = run(
            [
                "sweep",
                "--to",
                DEST,
                "--host",
                "127.0.0.1",
                "--port",
                str(port),
                "--fee-rate",
                "10",
                "--dry-run",
            ],
            input_bytes=WIF_G + "\n",
            home=home,
        )
        stop.set()
        thread.join(timeout=5)
        assert r.returncode == 1, r.stderr.decode()
        assert "insufficient funds" in r.stderr.decode()

    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except AssertionError as e:
        print("assertion failed:", e, file=sys.stderr)
        sys.exit(1)
