#!/usr/bin/env python3
"""Phase 4 btk node CLI tests. Live P2P only when BTK_RUN_NET=1."""

import hashlib
import json
import os
import pathlib
import socket
import struct
import subprocess
import sys
import threading

ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
BTK = ROOT / "bin" / "btk"

NODE_HELP = """btk node — Bitcoin P2P version handshake (IPv4 mainnet)

Usage:
  btk node --host HOST [--port 8333]

Connect to a Bitcoin P2P peer, send a version message (protocol
70015, user agent /Bitcoin-Toolkit:4.1.0/), print the peer's
version as a typed object, and close. Does not send verack. One
shot; not a key pipe. --host is required (no positional host).

IPv4 mainnet only (getaddrinfo AF_INET). Default port 8333. 15 s
timeout on connect and read. --host may include :port (one colon).
Combined with --port that is "port specified twice". --network is
ignored. Does not load ~/.btk/config.json.

Options:
  -h, --help             Show this help and exit
      --host HOST        IPv4 address or DNS name. Required. A
                         host:port form sets the port.
      --port PORT        TCP port. Default: 8333
      --verbose          Include raw P2P fields: addr_recv,
                         addr_trans, nonce (decimal string), and
                         services_bits
  -o, --out FORMAT       ndjson (default), json, or plain. plain
                         prints ip:port.

Examples:
  btk node --host seed.bitcoin.sipa.be
  btk node --host 127.0.0.1 --port 8333 --verbose
  btk node --host seed.bitcoin.sipa.be --out plain
"""

MAGIC = bytes.fromhex("f9beb4d9")


def hash256(data):
    return hashlib.sha256(hashlib.sha256(data).digest()).digest()


def compact_size(n):
    if n < 0xFD:
        return bytes([n])
    if n <= 0xFFFF:
        return b"\xfd" + struct.pack("<H", n)
    if n <= 0xFFFFFFFF:
        return b"\xfe" + struct.pack("<I", n)
    return b"\xff" + struct.pack("<Q", n)


def net_addr(ip="127.0.0.1", port=8333, services=0):
    parts = [int(x) for x in ip.split(".")]
    ipb = bytes(10) + b"\xff\xff" + bytes(parts)
    return struct.pack("<Q", services) + ipb + struct.pack(">H", port)


def version_payload(
    version=70016,
    services=1033,
    timestamp=1682030946,
    ua="/Satoshi:24.0.1/",
    height=786299,
    relay=True,
    nonce=0x1122334455667788,
    recv_ip="127.0.0.1",
    trans_ip="8.8.8.8",
    recv_port=8333,
    trans_port=8333,
):
    ua_b = ua.encode()
    return (
        struct.pack("<i", version)
        + struct.pack("<Q", services)
        + struct.pack("<q", timestamp)
        + net_addr(recv_ip, recv_port, 0)
        + net_addr(trans_ip, trans_port, 1)
        + struct.pack("<Q", nonce)
        + compact_size(len(ua_b))
        + ua_b
        + struct.pack("<i", height)
        + bytes([1 if relay else 0])
    )


def p2p_message(command, payload):
    cmd = command.encode().ljust(12, b"\x00")
    return MAGIC + cmd + struct.pack("<I", len(payload)) + hash256(payload)[:4] + payload


def recvall(conn, n):
    buf = b""
    while len(buf) < n:
        chunk = conn.recv(n - len(buf))
        if not chunk:
            break
        buf += chunk
    return buf


def start_peer(payload, extra=None):
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind(("127.0.0.1", 0))
    srv.listen(1)
    srv.settimeout(5)
    port = srv.getsockname()[1]
    received = {}

    def run():
        try:
            conn, _ = srv.accept()
            with conn:
                conn.settimeout(5)
                hdr = recvall(conn, 24)
                if len(hdr) < 24:
                    received["error"] = "short header"
                    return
                length = struct.unpack_from("<I", hdr, 16)[0]
                body = recvall(conn, length)
                received["header"] = hdr
                received["payload"] = body
                conn.sendall(p2p_message("version", payload))
                if extra:
                    conn.sendall(extra)
                conn.settimeout(0.2)
                try:
                    received["extra"] = recvall(conn, 1)
                except socket.timeout:
                    received["extra"] = b""
        except Exception as exc:  # pragma: no cover
            received["error"] = exc
        finally:
            srv.close()

    t = threading.Thread(target=run, daemon=True)
    t.start()
    return port, t, received


def run(args, input_bytes=None, timeout=5):
    if isinstance(input_bytes, str):
        input_bytes = input_bytes.encode()
    return subprocess.run(
        [str(BTK), *args],
        input=input_bytes,
        capture_output=True,
        timeout=timeout,
        cwd=str(ROOT),
    )


def ndjson(stdout):
    lines = [ln for ln in stdout.decode().splitlines() if ln]
    return [json.loads(ln) for ln in lines]


def expect_ok(args, timeout=10):
    r = run(args, timeout=timeout)
    assert r.returncode == 0, (r.returncode, r.stderr.decode(), r.stdout.decode())
    return r


def expect_err(args, needle, timeout=5):
    r = run(args, timeout=timeout)
    assert r.returncode == 1, (r.returncode, r.stderr.decode(), r.stdout.decode())
    err = r.stderr.decode()
    assert needle in err, err
    assert "btk node:" in err, err
    return r


def parse_our_version(payload):
    version, services, timestamp = struct.unpack_from("<iQq", payload, 0)
    off = 4 + 8 + 8 + 26 + 26 + 8
    ua_len = payload[off]
    off += 1
    ua = payload[off : off + ua_len].decode()
    off += ua_len
    height = struct.unpack_from("<i", payload, off)[0]
    relay = payload[off + 4]
    return {
        "version": version,
        "services": services,
        "timestamp": timestamp,
        "user_agent": ua,
        "height": height,
        "relay": relay,
    }


def handshake(args_extra=None, peer_payload=None):
    payload = peer_payload if peer_payload is not None else version_payload()
    port, thread, received = start_peer(payload)
    args = ["node", "--host", "127.0.0.1", "--port", str(port)]
    if args_extra:
        args.extend(args_extra)
    r = expect_ok(args)
    thread.join(timeout=5)
    assert "error" not in received, received.get("error")
    return r, received, port


def main():
    r = run(["node", "--help"])
    assert r.returncode == 0, r.stderr.decode()
    assert r.stdout.decode() == NODE_HELP

    expect_err(["node"], "missing host")
    expect_err(["node", "--host"], "missing option argument")
    expect_err(["node", "seed.bitcoin.sipa.be"], "provide input on stdin")
    expect_err(["node", "--host", "127.0.0.1", "leftover"], "provide input on stdin")
    expect_err(["node", "--stream", "--host", "127.0.0.1"], "node does not stream")
    expect_err(["node", "--count", "2", "--host", "127.0.0.1"], "unknown option '--count'")
    expect_err(["node", "--from", "wif", "--host", "127.0.0.1"], "unknown option '--from'")
    expect_err(["node", "--host", "127.0.0.1", "--port", "0"], "invalid --port")
    expect_err(["node", "--host", "127.0.0.1", "--port", "65536"], "invalid --port")
    expect_err(["node", "--host", "127.0.0.1:8333", "--port", "8333"], "port specified twice")
    expect_err(["node", "--host", "127.0.0.1:abc"], "invalid port")
    expect_err(["node", "--host", "1:2:3"], "invalid host")
    expect_err(["node", "--host", ":8333"], "missing host")

    r, received, port = handshake()
    obj = ndjson(r.stdout)[0]
    assert obj["type"] == "node"
    assert obj["host"] == "127.0.0.1"
    assert obj["ip"] == "127.0.0.1"
    assert obj["port"] == port
    assert obj["protocol"] == 70016
    assert obj["user_agent"] == "/Satoshi:24.0.1/"
    assert obj["height"] == 786299
    assert obj["services"] == ["NODE_NETWORK", "NODE_WITNESS", "NODE_NETWORK_LIMITED"]
    assert obj["relay"] is True
    assert obj["timestamp"] == 1682030946
    assert "raw" not in obj

    hdr = received["header"]
    assert hdr[:4] == MAGIC
    assert hdr[4:16] == b"version" + b"\x00" * 5
    ours = parse_our_version(received["payload"])
    assert ours["version"] == 70015
    assert ours["services"] == 0
    assert ours["user_agent"] == "/Bitcoin-Toolkit:4.1.0/"
    assert ours["height"] == 0
    assert ours["relay"] == 0
    assert received["extra"] == b""

    r, received, port = handshake(["--verbose"])
    obj = ndjson(r.stdout)[0]
    raw = obj["raw"]
    assert raw["nonce"] == str(0x1122334455667788)
    assert raw["services_bits"] == 1033
    assert raw["addr_recv"]["ip"] == "127.0.0.1"
    assert raw["addr_recv"]["port"] == 8333
    assert raw["addr_trans"]["ip"] == "8.8.8.8"
    assert raw["addr_trans"]["port"] == 8333

    payload = version_payload()
    mock_port, thread, received = start_peer(payload)
    r = expect_ok(["node", "--host", f"127.0.0.1:{mock_port}", "--out", "plain"])
    thread.join(timeout=5)
    assert r.stdout.decode().strip() == f"127.0.0.1:{mock_port}"

    odd = version_payload(services=(1 << 0) | (1 << 5))
    r, _, _ = handshake(peer_payload=odd)
    obj = ndjson(r.stdout)[0]
    assert obj["services"] == ["NODE_NETWORK", "BIT_5"]

    if os.environ.get("BTK_RUN_NET") == "1":
        r = expect_ok(["node", "--host", "seed.bitcoin.sipa.be"], timeout=25)
        live = ndjson(r.stdout)[0]
        assert live["type"] == "node"
        assert live["host"] == "seed.bitcoin.sipa.be"
        assert live["port"] == 8333
        assert isinstance(live["protocol"], int) and live["protocol"] > 0
        assert live["user_agent"].startswith("/")
        assert live["ip"].count(".") == 3
        assert "services" in live
        r = expect_ok(
            ["node", "--host", "seed.bitcoin.sipa.be", "--out", "plain"], timeout=25
        )
        plain = r.stdout.decode().strip()
        assert ":" in plain

    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except AssertionError as e:
        import traceback

        traceback.print_exc()
        print("assertion failed:", e, file=sys.stderr)
        sys.exit(1)
