#!/usr/bin/env python3
"""Phase 0 dispatcher stubs: help, version, unknown command."""

import json
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
BTK = ROOT / "bin" / "btk"


def run(args, input_bytes=None):
    return subprocess.run(
        [str(BTK), *args],
        input=input_bytes,
        capture_output=True,
        cwd=str(ROOT),
    )


def main():
    r = run([])
    assert r.returncode == 1, r.returncode
    err = r.stderr.decode()
    assert "Bitcoin Toolkit 4.0.4" in err
    assert r.stdout == b""

    r = run(["--help"])
    assert r.returncode == 0
    out = r.stdout.decode()
    assert "Bitcoin Toolkit 4.0.4" in out
    assert "btk <command> [options]" in out
    assert "  btk --help\n" in out
    assert "  btk --version\n" in out
    assert "-h, --help" in out
    assert "-V, --version" in out
    assert "--config PATH" not in out
    assert "-n, --network" not in out
    assert "-o, --out" not in out
    assert "--in FORMAT" not in out
    assert "-s, --stream" not in out
    assert "-c, --count" not in out

    r = run(["--version"])
    assert r.returncode == 0
    obj = json.loads(r.stdout.decode().strip())
    assert obj["type"] == "version"
    assert obj["version"] == "4.0.4"
    assert obj["secp256k1"] is True
    assert "leveldb" in obj

    r = run(["--version", "--out", "plain"])
    assert r.returncode == 0
    assert r.stdout.decode().strip() == "4.0.4"

    r = run(["foo"])
    assert r.returncode == 1
    err = r.stderr.decode()
    assert "unknown command 'foo'" in err
    assert "btk --help" in err

    r = run(["--help"])
    out = r.stdout.decode()
    assert "privkey" in out
    assert "pubkey" in out
    assert "address" in out
    assert "node" in out
    assert "balance" in out
    assert "config" in out
    assert "inflow" not in out
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except AssertionError as e:
        print("assertion failed:", e, file=sys.stderr)
        sys.exit(1)
