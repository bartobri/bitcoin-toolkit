#!/usr/bin/env python3
"""Hidden btk inflow CLI tests. Offline mock RPC only. Not user-facing."""

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

P2WPKH_G = "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4"
P2PKH_G = "1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH"
WIF_G_COMP_MAIN = "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn"

A10 = bytes.fromhex(
    "0200000000010100000000000000000000000000000000000000000000000000000000000000000000000000ffffffff01a086010000000000160014751e76e8199196d454941c45d1b3a323f1433bd60248300000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001210279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f8179800000000"
)

A10_TXID_INTERNAL = bytes.fromhex(
    "3c58a2ad2dfc2f132e6dd137844f6e6bd749e33672f5e24d5109cea990c282a8"
)

T0 = 1700000000
T1 = 1700001000
LAST0 = "2023-11-14 22:13:20 UTC"
LAST1 = "2023-11-14 22:30:00 UTC"
LAST2 = "2023-11-14 22:30:50 UTC"


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


def make_block(txs, nonce=0, block_time=0):
    header = (
        struct.pack("<i", 1)
        + bytes(32)
        + bytes(32)
        + struct.pack("<III", block_time, 0, nonce)
    )
    body = compact_size(len(txs)) + b"".join(txs)
    return header + body


def display_hash(block):
    return hash256(block[:80])[::-1].hex()


def spend_to_p2pkh():
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


def pay_p2wpkh(value=50000, prev_byte=1):
    spk = bytes.fromhex("0014751e76e8199196d454941c45d1b3a323f1433bd6")
    prev = bytes([prev_byte]) + bytes(31)
    return (
        struct.pack("<i", 1)
        + compact_size(1)
        + prev
        + struct.pack("<I", 0)
        + compact_size(0)
        + struct.pack("<I", 0xFFFFFFFF)
        + compact_size(1)
        + struct.pack("<q", value)
        + compact_size(len(spk))
        + spk
        + struct.pack("<I", 0)
    )


def start_rpc(blocks, auth=None):
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
    r = run(["inflow", "--help"])
    assert r.returncode == 0, r.stderr.decode()
    assert r.stdout == b""
    assert r.stderr == b""

    r = run(["--help"])
    assert r.returncode == 0
    out = r.stdout.decode()
    assert "inflow" not in out
    assert "privkey" in out
    assert "balance" in out

    with tempfile.TemporaryDirectory() as home:
        r = run(["inflow"], input_bytes=P2WPKH_G + "\n", home=home)
        assert r.returncode == 1, r.stderr.decode()
        err = r.stderr.decode()
        assert err.startswith("btk inflow:"), err
        assert "inflow database not found (run btk inflow --sync)" in err

        r = run(["inflow", "1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH"], home=home)
        assert r.returncode == 1
        assert "provide input on stdin" in r.stderr.decode()

        r = run(["inflow", "--path", "/tmp"], home=home)
        assert r.returncode == 1
        assert "unknown option '--path'" in r.stderr.decode()

        r = run(["inflow", "--force"], home=home)
        assert r.returncode == 1
        assert "--force requires --sync" in r.stderr.decode()

        r = run(["inflow"], input_bytes="not-an-address\n", home=home)
        assert r.returncode == 1
        assert "not a bitcoin address" in r.stderr.decode()

        r = run(
            ["inflow"],
            input_bytes="tb1qw508d6qejxtdg4y5r3zarvary0c5xw7kxpjzsx\n",
            home=home,
        )
        assert r.returncode == 1
        assert "not a bitcoin address" in r.stderr.decode()

        r = run(["inflow"], input_bytes=b"", home=home)
        assert r.returncode == 0
        assert r.stdout == b""

        blk0 = make_block([A10], nonce=1, block_time=T0)
        port, stop, thread, errors = start_rpc([blk0])
        r = run(
            ["inflow", "--sync", "--host", "127.0.0.1", "--port", str(port), "--rpc-auth", "u:p"],
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

        r = run(["inflow"], input_bytes=P2WPKH_G + "\n", home=home)
        assert r.returncode == 0, r.stderr.decode()
        obj = ndjson(r.stdout)[0]
        assert obj["type"] == "inflow"
        assert obj["address"] == P2WPKH_G
        assert obj["sats"] == 100000
        assert obj["count"] == 1
        assert obj["last"] == LAST0

        r = run(["inflow", "--out", "plain"], input_bytes=P2WPKH_G + "\n", home=home)
        assert r.returncode == 0
        assert r.stdout.decode().strip() == "100000"

        r = run(["inflow"], input_bytes=P2PKH_G + "\n", home=home)
        assert r.returncode == 0
        zero = ndjson(r.stdout)[0]
        assert zero["sats"] == 0
        assert zero["count"] == 0
        assert zero["last"] == ""

        r = run(["inflow", "--skip-zero"], input_bytes=P2WPKH_G + "\n", home=home)
        assert r.returncode == 0, r.stderr.decode()
        assert ndjson(r.stdout)[0]["sats"] == 100000

        r = run(["inflow", "--skip-zero"], input_bytes=P2PKH_G + "\n", home=home)
        assert r.returncode == 0, r.stderr.decode()
        assert r.stdout == b""

        mixed = (
            json.dumps({"type": "address", "data": P2PKH_G})
            + "\n"
            + json.dumps({"type": "address", "data": P2WPKH_G})
            + "\n"
        )
        r = run(["inflow", "--skip-zero"], input_bytes=mixed, home=home)
        assert r.returncode == 0, r.stderr.decode()
        objs = ndjson(r.stdout)
        assert len(objs) == 1
        assert objs[0]["address"] == P2WPKH_G

        r = run(
            ["inflow", "--skip-zero", "--out", "plain"],
            input_bytes=P2PKH_G + "\n" + P2WPKH_G + "\n",
            home=home,
        )
        assert r.returncode == 0, r.stderr.decode()
        assert r.stdout.decode() == "100000\n"

        r = run(["inflow", "--sync", "--skip-zero"], home=home)
        assert r.returncode == 1
        assert "cannot combine --sync and --skip-zero" in r.stderr.decode()

        r = run(
            ["inflow"],
            input_bytes=json.dumps({"type": "address", "data": P2WPKH_G}) + "\n",
            home=home,
        )
        assert r.returncode == 0
        assert ndjson(r.stdout)[0]["sats"] == 100000

        r = run(
            ["inflow"],
            input_bytes=json.dumps({"type": "inflow", "address": P2WPKH_G, "sats": 1}) + "\n",
            home=home,
        )
        assert r.returncode == 0
        assert ndjson(r.stdout)[0]["sats"] == 100000

        r = run(
            ["inflow"],
            input_bytes=json.dumps({"type": "balance", "address": P2WPKH_G, "sats": 1}) + "\n",
            home=home,
        )
        assert r.returncode == 0
        assert ndjson(r.stdout)[0]["sats"] == 100000

        r = run(
            ["inflow"],
            input_bytes=json.dumps({"type": "privkey", "data": "x"}) + "\n",
            home=home,
        )
        assert r.returncode == 1
        assert "expected an address" in r.stderr.decode()

        r = run(["inflow", "--from", "address"], input_bytes=P2WPKH_G + "\n", home=home)
        assert r.returncode == 0
        assert ndjson(r.stdout)[0]["sats"] == 100000

        r = run(["inflow", "--from", "wif"], input_bytes=P2WPKH_G + "\n", home=home)
        assert r.returncode == 1
        assert "invalid --from" in r.stderr.decode()

        r = run(["inflow", "--sync", "--source"], home=home)
        assert r.returncode == 1
        assert "cannot combine --sync and --source" in r.stderr.decode()

        r = run(["inflow", "--source"], input_bytes=P2WPKH_G + "\n", home=home)
        assert r.returncode == 0
        bare_src = ndjson(r.stdout)[0]["source"]
        assert bare_src["type"] == "address"
        assert bare_src["style"] == "p2wpkh"
        assert bare_src["network"] == "mainnet"
        assert bare_src["data"] == P2WPKH_G

        addr_r = run(["address", "--source"], input_bytes=WIF_G_COMP_MAIN + "\n", home=home)
        assert addr_r.returncode == 0, addr_r.stderr.decode()
        r = run(["inflow", "--source"], input_bytes=addr_r.stdout, home=home)
        assert r.returncode == 0, r.stderr.decode()
        sourced = ndjson(r.stdout)[0]
        assert sourced["address"] == P2WPKH_G
        assert sourced["source"]["type"] == "address"
        assert sourced["source"]["source"]["type"] == "privkey"

        r = run(["inflow"], input_bytes=addr_r.stdout, home=home)
        assert r.returncode == 0
        assert "source" not in ndjson(r.stdout)[0]

        # Incremental spend: inflow keeps received sats (unlike balance)
        blk1 = make_block([spend_to_p2pkh()], nonce=2, block_time=T1)
        port, stop, thread, errors = start_rpc([blk0, blk1])
        r = run(
            ["inflow", "--sync", "--host", "127.0.0.1", "--port", str(port)],
            home=home,
            timeout=15,
        )
        stop.set()
        thread.join(timeout=5)
        assert r.returncode == 0, (r.stderr.decode(), errors)
        assert "complete: height 1" in r.stderr.decode()

        r = run(["inflow"], input_bytes=P2WPKH_G + "\n", home=home)
        assert r.returncode == 0
        kept = ndjson(r.stdout)[0]
        assert kept["sats"] == 100000
        assert kept["count"] == 1
        assert kept["last"] == LAST0
        r = run(["inflow"], input_bytes=P2PKH_G + "\n", home=home)
        assert r.returncode == 0
        got = ndjson(r.stdout)[0]
        assert got["sats"] == 100000
        assert got["count"] == 1
        assert got["last"] == LAST1

        # Second credit to the original address
        blk2 = make_block([pay_p2wpkh()], nonce=3, block_time=T1 + 50)
        port, stop, thread, errors = start_rpc([blk0, blk1, blk2])
        r = run(
            ["inflow", "--sync", "--host", "127.0.0.1", "--port", str(port)],
            home=home,
            timeout=15,
        )
        stop.set()
        thread.join(timeout=5)
        assert r.returncode == 0, (r.stderr.decode(), errors)
        r = run(["inflow"], input_bytes=P2WPKH_G + "\n", home=home)
        assert r.returncode == 0
        two = ndjson(r.stdout)[0]
        assert two["sats"] == 150000
        assert two["count"] == 2
        assert two["last"] == LAST2

        # --force rebuilds from genesis (only blk0)
        port, stop, thread, errors = start_rpc([blk0])
        r = run(
            ["inflow", "--sync", "--force", "--host", "127.0.0.1", "--port", str(port)],
            home=home,
            timeout=15,
        )
        stop.set()
        thread.join(timeout=5)
        assert r.returncode == 0, r.stderr.decode()
        r = run(["inflow"], input_bytes=P2WPKH_G + "\n", home=home)
        rebuilt = ndjson(r.stdout)[0]
        assert rebuilt["sats"] == 100000
        assert rebuilt["count"] == 1
        assert rebuilt["last"] == LAST0
        r = run(["inflow"], input_bytes=P2PKH_G + "\n", home=home)
        assert ndjson(r.stdout)[0]["sats"] == 0

        r = run(
            ["inflow", "--sync", "--from", "address", "--host", "127.0.0.1"],
            home=home,
        )
        assert r.returncode == 1
        assert "cannot combine --sync and --from" in r.stderr.decode()

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
            [str(BTK), "inflow", "--sync", "--host", "127.0.0.1", "--port", str(hang_port)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd=str(ROOT),
            env=env,
        )
        time.sleep(0.4)
        proc.send_signal(signal.SIGINT)
        try:
            _, errb = proc.communicate(timeout=5)
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.communicate()
            hang_stop.set()
            ht.join(timeout=2)
            raise AssertionError("SIGINT did not stop btk inflow --sync")
        hang_stop.set()
        ht.join(timeout=2)
        assert proc.returncode == 1, (proc.returncode, errb.decode())
        assert "interrupted" in errb.decode(), errb.decode()
        assert "btk inflow:" in errb.decode(), errb.decode()

    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except AssertionError as e:
        print("assertion failed:", e, file=sys.stderr)
        sys.exit(1)
