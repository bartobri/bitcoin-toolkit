#!/usr/bin/env python3
"""Phase 5 btk balance CLI tests. Offline mock RPC only."""

import hashlib
import json
import os
import pathlib
import signal
import socket
import struct
import subprocess
import sys
import tempfile
import threading
import time

ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
BTK = ROOT / "bin" / "btk"

BALANCE_HELP = """btk balance — local address → satoshi index

Usage:
  btk balance
  btk balance --sync [--host H] [--port P] [--rpc-auth USER:PASS]
                     [--force]

Query a local address-to-satoshi index, or synchronize it from
Bitcoin Core JSON-RPC. The index always lives at ~/.btk/balance.
Requires LevelDB at build time. Mainnet only.

Query is a transformer on stdin (no positional addresses). Items
are a typed address object (data), a typed balance object
(address), or a bare Base58Check / bech32 address. --from address
forces the bare-line parse. Leftover text is an error, not a hash.
Empty stdin is empty stdout, exit 0. A missing address is sats: 0.
A missing database is "balance database not found (run btk balance
--sync)". Query is read-only.

--sync walks Core JSON-RPC (getblockcount, getblockhash, getblock
hex). Missing or empty DB: walk 0…tip. Valid DB: walk
Mheight+1…tip (already at tip → complete, exit 0). Junk or a reorg:
rebuild with --sync --force. --force wipes the directory and walks
from genesis. Ctrl-C / SIGTERM abort within ~200 ms; the next
--sync continues from the last saved height. Progress is on stderr.

No cookie file. Use --rpc-auth user:pass or config rpc.auth.
--host / --port default to config rpc.host / rpc.port or
127.0.0.1 / 8332. A first mainnet sync needs a node that can serve
every historical block and can take days; later runs are
incremental.

Options:
  -h, --help             Show this help and exit
      --sync             Create the index or catch it up from RPC
      --force            With --sync, wipe ~/.btk/balance and walk
                         from genesis
      --host HOST        RPC host. Default: config rpc.host or
                         127.0.0.1. A host:port form sets the port.
      --port PORT        RPC port. Default: config rpc.port or 8332
      --rpc-auth USER:PASS
                         HTTP Basic credentials. Default: config
                         rpc.auth. No cookie file.
      --from address     Force bare stdin lines as addresses
      --source           Include the input item as a source object
      --config PATH      Config file. Default: $BTK_CONFIG, else
                         ~/.btk/config.json
  -o, --out FORMAT       ndjson (default), json, or plain. plain
                         prints the satoshi count.
      --in FORMAT        auto (default), ndjson, json, or plain

Examples:
  btk balance --sync
  printf '%s' 1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa | btk balance
  btk privkey --new | btk address | btk balance
  btk privkey --new | btk address --source | btk balance --source
"""

P2WPKH_G = "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4"
P2PKH_G = "1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH"
P2TR_UNTWEAKED_G = "bc1p0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7vqzk5jj0"
WIF_G_COMP_MAIN = "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn"
SECRET1 = "0000000000000000000000000000000000000000000000000000000000000001"

A10 = bytes.fromhex(
    "0200000000010100000000000000000000000000000000000000000000000000000000000000000000000000ffffffff01a086010000000000160014751e76e8199196d454941c45d1b3a323f1433bd60248300000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001210279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f8179800000000"
)

A10_TXID_INTERNAL = bytes.fromhex(
    "3c58a2ad2dfc2f132e6dd137844f6e6bd749e33672f5e24d5109cea990c282a8"
)


def compact_size(n):
    if n < 0xFD:
        return bytes([n])
    if n <= 0xFFFF:
        return b"\xfd" + struct.pack("<H", n)
    if n <= 0xFFFFFFFF:
        return b"\xfe" + struct.pack("<I", n)
    return b"\xff" + struct.pack("<Q", n)


def hash256(data):
    return hashlib.sha256(hashlib.sha256(data).digest()).digest()


def make_block(txs, nonce=0):
    header = (
        struct.pack("<i", 1)
        + bytes(32)
        + bytes(32)
        + struct.pack("<III", 0, 0, nonce)
    )
    body = compact_size(len(txs)) + b"".join(txs)
    return header + body


def display_hash(block):
    return hash256(block[:80])[::-1].hex()


def spend_to_p2pkh():
    # version 1, 1 in (A.10 vout 0), 1 out P2PKH of G, 100000 sats, no witness
    spk = bytes.fromhex("76a914751e76e8199196d454941c45d1b3a323f1433bd688ac")
    return (
        struct.pack("<i", 1)
        + compact_size(1)
        + A10_TXID_INTERNAL
        + struct.pack("<I", 0)
        + compact_size(0)
        + struct.pack("<I", 0xFFFFFFFF)
        + compact_size(1)
        + struct.pack("<q", 100000)
        + compact_size(len(spk))
        + spk
        + struct.pack("<I", 0)
    )


def start_rpc(blocks, auth=None):
    """blocks: list of raw block bytes, index = height."""
    hashes = [display_hash(b) for b in blocks]
    by_hash = {h: b for h, b in zip(hashes, blocks)}
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind(("127.0.0.1", 0))
    srv.listen(16)
    srv.settimeout(0.2)
    port = srv.getsockname()[1]
    stop = threading.Event()
    errors = []

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
        result = None
        if method == "getblockcount":
            result = len(blocks) - 1
        elif method == "getblockhash":
            h = int(params[0])
            result = hashes[h]
        elif method == "getblock":
            raw = by_hash[params[0]]
            result = raw.hex()
        else:
            raise RuntimeError("unknown method " + str(method))
        body = json.dumps({"result": result, "error": None, "id": req.get("id")}).encode()
        resp = (
            b"HTTP/1.0 200 OK\r\nContent-Type: application/json\r\nContent-Length: "
            + str(len(body)).encode()
            + b"\r\n\r\n"
            + body
        )
        conn.sendall(resp)

    def run():
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

    t = threading.Thread(target=run, daemon=True)
    t.start()
    return port, stop, t, errors


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


def main():
    r = run(["balance", "--help"])
    assert r.returncode == 0, r.stderr.decode()
    assert r.stdout.decode() == BALANCE_HELP, repr(r.stdout.decode())

    with tempfile.TemporaryDirectory() as home:
        r = run(["balance"], input_bytes=P2WPKH_G + "\n", home=home)
        assert r.returncode == 1, r.stderr.decode()
        assert "balance database not found (run btk balance --sync)" in r.stderr.decode()

        r = run(["balance", "1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH"], home=home)
        assert r.returncode == 1
        assert "provide input on stdin" in r.stderr.decode()

        r = run(["balance", "--path", "/tmp"], home=home)
        assert r.returncode == 1
        assert "unknown option '--path'" in r.stderr.decode()

        r = run(["balance", "--build"], home=home)
        assert r.returncode == 1
        assert "unknown option '--build'" in r.stderr.decode()

        r = run(["balance", "--update"], home=home)
        assert r.returncode == 1
        assert "unknown option '--update'" in r.stderr.decode()

        r = run(["balance", "--from-rpc"], home=home)
        assert r.returncode == 1
        assert "unknown option '--from-rpc'" in r.stderr.decode()

        r = run(["balance", "--from-chainstate"], home=home)
        assert r.returncode == 1
        assert "unknown option '--from-chainstate'" in r.stderr.decode()

        r = run(["balance", "--force"], home=home)
        assert r.returncode == 1
        assert "--force requires --sync" in r.stderr.decode()

        r = run(["balance"], input_bytes="not-an-address\n", home=home)
        assert r.returncode == 1
        assert "not a bitcoin address" in r.stderr.decode()

        r = run(
            ["balance"],
            input_bytes="tb1qw508d6qejxtdg4y5r3zarvary0c5xw7kxpjzsx\n",
            home=home,
        )
        assert r.returncode == 1
        assert "not a bitcoin address" in r.stderr.decode()

        r = run(["balance"], input_bytes=b"", home=home)
        assert r.returncode == 0
        assert r.stdout == b""

        blk0 = make_block([A10], nonce=1)
        port, stop, thread, errors = start_rpc([blk0])
        r = run(
            ["balance", "--sync", "--host", "127.0.0.1", "--port", str(port), "--rpc-auth", "u:p"],
            home=home,
            timeout=15,
        )
        stop.set()
        thread.join(timeout=5)
        assert r.returncode == 0, (r.stderr.decode(), r.stdout.decode(), errors)
        assert r.stdout == b""
        err = r.stderr.decode()
        assert "complete: height 0" in err, err
        assert "syncing:" in err

        r = run(["balance"], input_bytes=P2WPKH_G + "\n", home=home)
        assert r.returncode == 0, r.stderr.decode()
        obj = ndjson(r.stdout)[0]
        assert obj["type"] == "balance"
        assert obj["address"] == P2WPKH_G
        assert obj["sats"] == 100000

        r = run(["balance", "--out", "plain"], input_bytes=P2WPKH_G + "\n", home=home)
        assert r.returncode == 0
        assert r.stdout.decode().strip() == "100000"

        r = run(["balance"], input_bytes=P2PKH_G + "\n", home=home)
        assert r.returncode == 0
        assert ndjson(r.stdout)[0]["sats"] == 0

        r = run(
            ["balance"],
            input_bytes=json.dumps({"type": "address", "data": P2WPKH_G}) + "\n",
            home=home,
        )
        assert r.returncode == 0
        assert ndjson(r.stdout)[0]["sats"] == 100000

        many = (json.dumps({"type": "address", "data": P2WPKH_G}) + "\n") * 10
        r = run(["balance"], input_bytes=many, home=home)
        assert r.returncode == 0, r.stderr.decode()
        objs = ndjson(r.stdout)
        assert len(objs) == 10
        assert all(o["sats"] == 100000 for o in objs)

        r = run(
            ["balance"],
            input_bytes=json.dumps({"type": "balance", "address": P2WPKH_G, "sats": 1}) + "\n",
            home=home,
        )
        assert r.returncode == 0
        assert ndjson(r.stdout)[0]["sats"] == 100000

        r = run(
            ["balance"],
            input_bytes=json.dumps({"type": "privkey", "data": "x"}) + "\n",
            home=home,
        )
        assert r.returncode == 1
        assert "expected an address" in r.stderr.decode()

        r = run(["balance", "--from", "address"], input_bytes=P2WPKH_G + "\n", home=home)
        assert r.returncode == 0
        assert ndjson(r.stdout)[0]["sats"] == 100000

        r = run(["balance", "--from", "wif"], input_bytes=P2WPKH_G + "\n", home=home)
        assert r.returncode == 1
        assert "invalid --from" in r.stderr.decode()

        r = run(["balance", "--sync", "--source"], home=home)
        assert r.returncode == 1
        assert "cannot combine --sync and --source" in r.stderr.decode()

        addr_r = run(["address", "--source"], input_bytes=WIF_G_COMP_MAIN + "\n", home=home)
        assert addr_r.returncode == 0, addr_r.stderr.decode()
        r = run(["balance", "--source"], input_bytes=addr_r.stdout, home=home)
        assert r.returncode == 0, r.stderr.decode()
        sourced = ndjson(r.stdout)[0]
        assert sourced["address"] == P2WPKH_G
        assert sourced["sats"] == 100000
        assert sourced["source"]["type"] == "address"
        assert sourced["source"]["data"] == P2WPKH_G
        assert sourced["source"]["source"]["type"] == "privkey"
        assert sourced["source"]["source"]["data"] == WIF_G_COMP_MAIN

        r = run(["balance"], input_bytes=addr_r.stdout, home=home)
        assert r.returncode == 0
        assert "source" not in ndjson(r.stdout)[0]

        r = run(
            ["balance", "--source"],
            input_bytes=json.dumps({"type": "address", "data": P2WPKH_G}) + "\n",
            home=home,
        )
        assert r.returncode == 0
        no_key = ndjson(r.stdout)[0]
        assert no_key["source"]["type"] == "address"
        assert no_key["source"]["data"] == P2WPKH_G
        assert "source" not in no_key["source"]

        r = run(["balance", "--source"], input_bytes=P2WPKH_G + "\n", home=home)
        assert r.returncode == 0
        bare_src = ndjson(r.stdout)[0]["source"]
        assert bare_src["type"] == "address"
        assert bare_src["style"] == "p2wpkh"
        assert bare_src["network"] == "mainnet"
        assert bare_src["data"] == P2WPKH_G
        assert "source" not in bare_src

        r = run(
            ["balance", "--source", "--out", "plain"],
            input_bytes=P2WPKH_G + "\n",
            home=home,
        )
        assert r.returncode == 0
        assert r.stdout.decode().strip() == "100000"

        pub = {
            "type": "pubkey",
            "encoding": "hex",
            "network": "mainnet",
            "compressed": True,
            "data": "0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798",
            "source": {
                "type": "privkey",
                "encoding": "hex",
                "network": "mainnet",
                "compressed": True,
                "data": SECRET1,
            },
        }
        addr_from_pub = run(
            ["address", "--source"],
            input_bytes=json.dumps(pub) + "\n",
            home=home,
        )
        assert addr_from_pub.returncode == 0, addr_from_pub.stderr.decode()
        r = run(["balance", "--source"], input_bytes=addr_from_pub.stdout, home=home)
        assert r.returncode == 0, r.stderr.decode()
        chain = ndjson(r.stdout)[0]
        assert chain["source"]["type"] == "address"
        assert chain["source"]["source"]["type"] == "pubkey"
        assert chain["source"]["source"]["source"]["type"] == "privkey"
        assert chain["source"]["source"]["source"]["data"] == SECRET1

        # Already at tip: incremental no-op
        port, stop, thread, errors = start_rpc([blk0])
        r = run(
            ["balance", "--sync", "--host", "127.0.0.1", "--port", str(port)],
            home=home,
            timeout=15,
        )
        stop.set()
        thread.join(timeout=5)
        assert r.returncode == 0, r.stderr.decode()
        assert "complete: height 0" in r.stderr.decode()

        # Incremental: spend the A.10 output to P2PKH of G
        blk1 = make_block([spend_to_p2pkh()], nonce=2)
        port, stop, thread, errors = start_rpc([blk0, blk1])
        r = run(
            ["balance", "--sync", "--host", "127.0.0.1", "--port", str(port)],
            home=home,
            timeout=15,
        )
        stop.set()
        thread.join(timeout=5)
        assert r.returncode == 0, (r.stderr.decode(), errors)
        assert "complete: height 1" in r.stderr.decode()

        r = run(["balance"], input_bytes=P2WPKH_G + "\n", home=home)
        assert r.returncode == 0
        assert ndjson(r.stdout)[0]["sats"] == 0
        r = run(["balance"], input_bytes=P2PKH_G + "\n", home=home)
        assert r.returncode == 0
        assert ndjson(r.stdout)[0]["sats"] == 100000

        # --force rebuilds from genesis (only blk0 in this mock)
        port, stop, thread, errors = start_rpc([blk0])
        r = run(
            ["balance", "--sync", "--force", "--host", "127.0.0.1", "--port", str(port)],
            home=home,
            timeout=15,
        )
        stop.set()
        thread.join(timeout=5)
        assert r.returncode == 0, r.stderr.decode()
        r = run(["balance"], input_bytes=P2WPKH_G + "\n", home=home)
        assert ndjson(r.stdout)[0]["sats"] == 100000
        r = run(["balance"], input_bytes=P2PKH_G + "\n", home=home)
        assert ndjson(r.stdout)[0]["sats"] == 0

        # Same-block spend: child must debit the parent created in this block
        blk_same = make_block([A10, spend_to_p2pkh()], nonce=3)
        port, stop, thread, errors = start_rpc([blk_same])
        r = run(
            ["balance", "--sync", "--force", "--host", "127.0.0.1", "--port", str(port)],
            home=home,
            timeout=15,
        )
        stop.set()
        thread.join(timeout=5)
        assert r.returncode == 0, (r.stderr.decode(), errors)
        r = run(["balance"], input_bytes=P2WPKH_G + "\n", home=home)
        assert r.returncode == 0
        assert ndjson(r.stdout)[0]["sats"] == 0
        r = run(["balance"], input_bytes=P2PKH_G + "\n", home=home)
        assert r.returncode == 0
        assert ndjson(r.stdout)[0]["sats"] == 100000

        r = run(
            ["balance", "--sync", "--from", "address", "--host", "127.0.0.1"],
            home=home,
        )
        assert r.returncode == 1
        assert "cannot combine --sync and --from" in r.stderr.decode()

        # Ctrl-C / SIGINT must abort a stuck RPC --sync.
        hang = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        hang.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        hang.bind(("127.0.0.1", 0))
        hang.listen(1)
        hang_port = hang.getsockname()[1]
        hang_stop = threading.Event()

        def hang_run():
            hang.settimeout(0.2)
            try:
                while not hang_stop.is_set():
                    try:
                        conn, _ = hang.accept()
                    except socket.timeout:
                        continue
                    try:
                        while not hang_stop.is_set():
                            time.sleep(0.05)
                    finally:
                        conn.close()
            finally:
                hang.close()

        ht = threading.Thread(target=hang_run, daemon=True)
        ht.start()
        env = os.environ.copy()
        env["HOME"] = home
        proc = subprocess.Popen(
            [str(BTK), "balance", "--sync", "--host", "127.0.0.1", "--port", str(hang_port)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd=str(ROOT),
            env=env,
        )
        time.sleep(0.4)
        proc.send_signal(signal.SIGINT)
        try:
            _, err = proc.communicate(timeout=5)
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.communicate()
            hang_stop.set()
            ht.join(timeout=2)
            raise AssertionError("SIGINT did not stop btk balance --sync")
        hang_stop.set()
        ht.join(timeout=2)
        assert proc.returncode == 1, (proc.returncode, err.decode())
        assert "interrupted" in err.decode(), err.decode()

    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except AssertionError as e:
        print("assertion failed:", e, file=sys.stderr)
        sys.exit(1)
