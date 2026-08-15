#!/usr/bin/env python3
"""Phase 1 btk privkey CLI tests."""

import json
import pathlib
import select
import signal
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
BTK = ROOT / "bin" / "btk"

SECRET1 = "0000000000000000000000000000000000000000000000000000000000000001"
WIF_G_COMP_MAIN = "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn"
WIF_G_UNC_MAIN = "5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreAnchuDf"
WIF_G_COMP_TEST = "cMahea7zqjxrtgAbB7LSGbcQUr1uX1ojuat9jZodMN87JcbXMTcA"
WIF_G_UNC_TEST = "91avARGdfge8E4tZfYLoxeJ5sGBdNJQH4kvjJoQFacbgwmaKkrx"

WIKI_HEX = "18e14a7b6a307f426a94f8114701e7c8e774e7f9a47e2c2035db29a206321725"
WIKI_WIF_COMP_MAIN = "Kx45GeUBSMPReYQwgXiKhG9FzNXrnCeutJp4yjTd5kKxCitadm3C"
WIKI_WIF_UNC_MAIN = "5J1F7GHadZG3sCCKHCwg8Jvys9xUbFsjLnGec4H125Ny1V9nR6V"

WIF_WIKI_HEX = "0c28fca386c7a227600b2fe50b7cae11ec86d3bf1fbe471be89827e19d72aa1d"
WIF_WIKI_UNC = "5HueCGU8rMjxEXxiPuD5BDku4MkFqeZyd4dZ1jvhTVqvbTLvyTJ"
WIF_WIKI_COMP = "KwdMAjGmerYanjeui5SHS7JkmpZvVipYvB2LJGU1ZxJwYvP98617"

TEST01_WIF = "Kzh1d5pXSZLtwsgENakrfCjuGy9txPEb3aEb2y8yyZo65qDs8bTu"
TEST01_UNC = "5JbtoEnCt6yAWCUvKwKYeCitigTV5qzTHtwHKa7Lhuk4sYDnTpP"
TEST01_COMP_TEST = "cR415zpNsd3A7K9VkzZz2XExuCTJcqLH7cP49PbVUgT6LaJiWb1Q"
TEST01_UNC_TEST = "92NXNybkUL3JUFzCxHDTWoGrNLpCF1XedqoEQCTr3eV7ebgGHp5"
SECRET_PASSPHRASE_WIF = "L1Cf21MBhiZX9QFTAhN3PGJkyvQzN4CuHwhasHsdV9tkEfiiB8Ug"

ZERO = "00" * 32
N = "fffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141"
N_PLUS_1 = "fffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364142"
N_DEC = "115792089237316195423570985008687907852837564279074904382605163141518161494337"
WIF_WIKI_DEC = "5500171714335001507730457227127633683517613019341760098818554179534751705629"

PRIVKEY_HELP = """btk privkey — create or convert private keys

Usage:
  btk privkey --new [--count N] [--stream]
              [--encoding wif|hex|dec] [--network mainnet|testnet]
              [--compressed | --uncompressed]
  btk privkey [--encoding wif|hex|dec] [--network mainnet|testnet]
              [--compressed | --uncompressed]
              [--from wif|hex|dec|text|file]

  --new              CSPRNG key in [1, n-1]
  --encoding         Output wif (default), hex, or dec
  --network          mainnet (default) or testnet; WIF version byte
  --compressed       Set the WIF/pubkey compression flag (default)
  --uncompressed     Clear the flag; both flags emit two objects
  --from             Force stdin type (default: guess)
  --count N          With --new, emit N keys
  --stream           With --new, emit until SIGINT
  --out ndjson|json|plain
  --in  auto|ndjson|json|plain

Input is stdin only (no positional keys). Guess order: WIF, 64-char
hex, decimal, text (SHA-256), then binary file (whole stdin).
--from text|file overrides the guess (e.g. printf 1 | btk privkey --from text).
"""


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


def expect_ok(args, input_bytes=None):
    r = run(args, input_bytes=input_bytes)
    assert r.returncode == 0, (r.returncode, r.stderr.decode(), r.stdout.decode())
    return r


def expect_err(args, needle, input_bytes=None):
    r = run(args, input_bytes=input_bytes)
    assert r.returncode == 1, r.returncode
    err = r.stderr.decode()
    assert needle in err, err
    assert "btk privkey:" in err, err
    return r


def plain(args, data):
    r = expect_ok(args, input_bytes=data)
    return r.stdout.decode().strip()


def main():
    r = expect_ok(["privkey", "--new", "--out", "plain"])
    wif = r.stdout.decode().strip()
    assert wif[0] in ("K", "L"), wif
    assert "\n" not in wif

    hx = plain(["privkey", "--encoding", "hex", "--out", "plain"], wif)
    assert len(hx) == 64 and hx == hx.lower()
    assert plain(["privkey", "--out", "plain"], hx) == wif

    assert plain(["privkey", "--out", "plain"], SECRET1) == WIF_G_COMP_MAIN
    assert plain(["privkey", "--uncompressed", "--out", "plain"], SECRET1) == WIF_G_UNC_MAIN
    assert plain(["privkey", "--network", "testnet", "--out", "plain"], SECRET1) == WIF_G_COMP_TEST
    assert (
        plain(["privkey", "--network", "testnet", "--uncompressed", "--out", "plain"], SECRET1)
        == WIF_G_UNC_TEST
    )

    assert plain(["privkey", "--encoding", "hex", "--out", "plain"], WIF_G_COMP_MAIN) == SECRET1

    assert plain(["privkey", "--out", "plain"], WIKI_HEX) == WIKI_WIF_COMP_MAIN
    assert plain(["privkey", "--uncompressed", "--out", "plain"], WIKI_HEX) == WIKI_WIF_UNC_MAIN
    assert plain(["privkey", "--encoding", "hex", "--out", "plain"], WIKI_WIF_COMP_MAIN) == WIKI_HEX

    assert plain(["privkey", "--uncompressed", "--out", "plain"], WIF_WIKI_HEX) == WIF_WIKI_UNC
    assert plain(["privkey", "--out", "plain"], WIF_WIKI_HEX) == WIF_WIKI_COMP

    assert plain(["privkey", "--from", "text", "--out", "plain"], "test01") == TEST01_WIF
    assert plain(["privkey", "--from", "text", "--uncompressed", "--out", "plain"], "test01") == TEST01_UNC
    assert (
        plain(["privkey", "--from", "text", "--network", "testnet", "--out", "plain"], "test01")
        == TEST01_COMP_TEST
    )
    assert (
        plain(
            ["privkey", "--from", "text", "--network", "testnet", "--uncompressed", "--out", "plain"],
            "test01",
        )
        == TEST01_UNC_TEST
    )
    assert (
        plain(["privkey", "--from", "text", "--out", "plain"], "Secret Passphrase")
        == SECRET_PASSPHRASE_WIF
    )

    expect_err(["privkey", "--out", "plain"], "out of range", ZERO)
    expect_err(["privkey", "--out", "plain"], "out of range", N)
    expect_err(["privkey", "--out", "plain"], "out of range", N_PLUS_1)

    r = expect_ok(["privkey", "--new", "--count", "3"])
    objs = ndjson(r.stdout)
    assert len(objs) == 3
    datas = [o["data"] for o in objs]
    assert len(set(datas)) == 3
    for o in objs:
        assert o["type"] == "privkey"
        assert o["encoding"] == "wif"
        assert o["network"] == "mainnet"
        assert o["compressed"] is True

    pipe = (WIF_G_COMP_MAIN + "\n" + WIKI_WIF_COMP_MAIN + "\n").encode()
    r = expect_ok(["privkey", "--encoding", "hex"], input_bytes=pipe)
    objs = ndjson(r.stdout)
    assert [o["data"] for o in objs] == [SECRET1, WIKI_HEX]

    r = expect_ok(["privkey"], input_bytes=b"")
    assert r.stdout == b""

    expect_err(["privkey", "--new", "--from", "text"], "cannot combine --new and --from")
    expect_err(["privkey", "--count", "3"], "--count requires --new")
    expect_err(["privkey", "--stream"], "--stream requires --new")
    expect_err(["privkey", "--count", "0"], "invalid --count")
    expect_err(["privkey", "--from", "foo"], "invalid --from")
    expect_err(["privkey", SECRET1], "provide input on stdin")

    hashed = plain(["privkey", "--out", "plain"], "not-a-key")
    assert plain(["privkey", "--from", "text", "--out", "plain"], "not-a-key") == hashed

    expect_err(
        ["privkey"],
        "invalid WIF checksum",
        "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWo",
    )
    expect_err(["privkey"], "out of range", "00")

    assert plain(["privkey", "--encoding", "dec", "--out", "plain"], SECRET1) == "1"
    assert plain(["privkey", "--out", "plain"], "1") == WIF_G_COMP_MAIN
    assert plain(["privkey", "--from", "text", "--out", "plain"], "1") != WIF_G_COMP_MAIN
    assert plain(["privkey", "--encoding", "hex", "--out", "plain"], "001") == SECRET1
    assert plain(["privkey", "--encoding", "dec", "--out", "plain"], WIF_WIKI_UNC) == WIF_WIKI_DEC
    expect_err(["privkey"], "out of range", N_DEC)

    plus = plain(["privkey", "--out", "plain"], "+1")
    assert plus == plain(["privkey", "--from", "text", "--out", "plain"], "+1")

    sixty_four_ones = "1" * 64
    hex_from_bare = plain(["privkey", "--encoding", "hex", "--out", "plain"], sixty_four_ones)
    r = expect_ok(
        ["privkey", "--encoding", "hex", "--out", "plain"],
        input_bytes=json.dumps(
            {"type": "privkey", "encoding": "dec", "data": sixty_four_ones}
        ).encode()
        + b"\n",
    )
    assert r.stdout.decode().strip() != hex_from_bare
    assert (
        plain(["privkey", "--from", "dec", "--encoding", "hex", "--out", "plain"], sixty_four_ones)
        != hex_from_bare
    )

    assert (
        plain(["privkey", "--out", "plain"], '{"type":"privkey","encoding":"dec","data":"1"}\n')
        == WIF_G_COMP_MAIN
    )
    expect_err(["privkey"], "expected a privkey", '{"type":"pubkey","data":"02aa"}\n')

    r = expect_ok(
        ["privkey", "--encoding", "hex"],
        input_bytes=json.dumps(
            {
                "type": "privkey",
                "encoding": "wif",
                "network": "mainnet",
                "compressed": True,
                "data": WIF_G_COMP_MAIN,
            }
        ).encode()
        + b"\n",
    )
    assert ndjson(r.stdout)[0]["data"] == SECRET1

    r = expect_ok(
        ["privkey", "--out", "plain"],
        input_bytes=json.dumps(
            {
                "type": "privkey",
                "encoding": "hex",
                "network": "testnet",
                "compressed": False,
                "data": SECRET1,
            }
        ).encode()
        + b"\n",
    )
    assert r.stdout.decode().strip() == WIF_G_UNC_TEST

    r = expect_ok(["privkey", "--compressed", "--uncompressed", "--out", "plain"], SECRET1)
    lines = [ln for ln in r.stdout.decode().splitlines() if ln]
    assert lines == [WIF_G_COMP_MAIN, WIF_G_UNC_MAIN]

    r = expect_ok(["privkey", "--out", "json"], SECRET1)
    obj = json.loads(r.stdout.decode())
    assert obj["type"] == "privkey"
    assert obj["data"] == WIF_G_COMP_MAIN

    assert ndjson(expect_ok(["privkey", "--encoding", "hex"], SECRET1.upper()).stdout)[0]["data"] == SECRET1

    assert plain(["privkey", "--from", "file", "--out", "plain"], b"test01") == TEST01_WIF
    blob = b"\xff\xd8\xff\x00not-a-key"
    guessed = plain(["privkey", "--out", "plain"], blob)
    forced = plain(["privkey", "--from", "file", "--out", "plain"], blob)
    assert guessed == forced
    zip_magic = b"PK\x03\x04" + b"\x00\x01\x02\x03"
    assert plain(["privkey", "--out", "plain"], zip_magic) == plain(
        ["privkey", "--from", "file", "--out", "plain"], zip_magic
    )

    r = expect_ok(["privkey", "--help"])
    assert r.stdout.decode() == PRIVKEY_HELP

    proc = subprocess.Popen(
        [str(BTK), "privkey", "--new", "--stream"],
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        cwd=str(ROOT),
    )
    try:
        ready, _, _ = select.select([proc.stdout], [], [], 2.0)
        assert ready, "stream did not emit within 2s"
        line = proc.stdout.readline()
        obj = json.loads(line)
        assert obj["type"] == "privkey"
        assert obj["encoding"] == "wif"
    finally:
        proc.send_signal(signal.SIGINT)
        try:
            proc.wait(timeout=2)
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.wait()

    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except AssertionError as e:
        print("assertion failed:", e, file=sys.stderr)
        sys.exit(1)
