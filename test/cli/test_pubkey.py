#!/usr/bin/env python3
"""Phase 2 btk pubkey CLI tests."""

import json
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
BTK = ROOT / "bin" / "btk"

SECRET1 = "0000000000000000000000000000000000000000000000000000000000000001"
SECRET6 = "0000000000000000000000000000000000000000000000000000000000000006"
WIKI_HEX = "18e14a7b6a307f426a94f8114701e7c8e774e7f9a47e2c2035db29a206321725"
WIF_WIKI_HEX = "0c28fca386c7a227600b2fe50b7cae11ec86d3bf1fbe471be89827e19d72aa1d"

WIF_G_COMP_MAIN = "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn"
WIF_G_UNC_MAIN = "5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreAnchuDf"
WIF_G_COMP_TEST = "cMahea7zqjxrtgAbB7LSGbcQUr1uX1ojuat9jZodMN87JcbXMTcA"
WIF_WIKI_COMP = "KwdMAjGmerYanjeui5SHS7JkmpZvVipYvB2LJGU1ZxJwYvP98617"

G_COMP = "0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798"
G_UNC = (
    "0479be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798"
    "483ada7726a3c4655da4fbfc0e1108a8fd17b448a68554199c47d08ffb10d4b8"
)
WIKI_COMP = "0250863ad64a87ae8a2fe83c1af1a8403cb53f53e486d8511dad8a04887e5b2352"
WIKI_UNC = (
    "0450863ad64a87ae8a2fe83c1af1a8403cb53f53e486d8511dad8a04887e5b2352"
    "2cd470243453a299fa9e77237716103abc11a1df38855ed6f2ee187e9c582ba6"
)
SEC6_COMP = "03fff97bd5755eeea420453a14355235d382f6472f8568a18b2f057a1460297556"
WIF_WIKI_COMP_PUB = "02d0de0aaeaefad02b8bdc8a01a1b8b11c696bd3d66a2c5f10780d95b7df42645c"

ZERO = "00" * 32
N = "fffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141"

PUBKEY_HELP = """btk pubkey — derive or recompress public keys

Usage:
  btk pubkey [--compressed | --uncompressed]
             [--from wif|hex|dec] [--source]

Items are a privkey or pubkey object, or a stdin line that is
already a key (WIF, hex priv, decimal, or hex pub). Guess order:
WIF, 64-char hex priv, decimal, 66/130-char hex pub. --from
overrides the guess (--from hex is 64-char priv or 66/130-char
pub). Leftover text is an error. Default compression follows the
input; flags override. Both flags emit two objects. Output is hex
(33 or 65 bytes). --source includes the parent key on the object.
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
    assert r.returncode == 1, (r.returncode, r.stderr.decode(), r.stdout.decode())
    err = r.stderr.decode()
    assert needle in err, err
    assert "btk pubkey:" in err, err
    return r


def plain(args, data):
    r = expect_ok(args, input_bytes=data)
    return r.stdout.decode().strip()


def obj(args, data):
    return ndjson(expect_ok(args, input_bytes=data).stdout)[0]


def main():
    assert plain(["pubkey", "--out", "plain"], SECRET1) == G_COMP
    assert plain(["pubkey", "--uncompressed", "--out", "plain"], SECRET1) == G_UNC
    assert plain(["pubkey", "--out", "plain"], WIF_G_COMP_MAIN) == G_COMP
    assert plain(["pubkey", "--out", "plain"], WIF_G_UNC_MAIN) == G_UNC
    assert plain(["pubkey", "--compressed", "--out", "plain"], WIF_G_UNC_MAIN) == G_COMP
    assert plain(["pubkey", "--out", "plain"], "1") == G_COMP
    assert plain(["pubkey", "--from", "dec", "--out", "plain"], "001") == G_COMP

    assert plain(["pubkey", "--out", "plain"], WIKI_HEX) == WIKI_COMP
    assert plain(["pubkey", "--uncompressed", "--out", "plain"], WIKI_HEX) == WIKI_UNC
    assert plain(["pubkey", "--out", "plain"], WIF_WIKI_COMP) == WIF_WIKI_COMP_PUB
    assert plain(["pubkey", "--out", "plain"], SECRET6) == SEC6_COMP

    assert plain(["pubkey", "--out", "plain"], G_COMP) == G_COMP
    assert plain(["pubkey", "--out", "plain"], G_UNC) == G_UNC
    assert plain(["pubkey", "--compressed", "--out", "plain"], G_UNC) == G_COMP
    assert plain(["pubkey", "--uncompressed", "--out", "plain"], G_COMP) == G_UNC
    assert plain(["pubkey", "--from", "hex", "--out", "plain"], G_COMP) == G_COMP
    assert plain(["pubkey", "--from", "hex", "--out", "plain"], SECRET1) == G_COMP

    r = expect_ok(["pubkey", "--compressed", "--uncompressed", "--out", "plain"], SECRET1)
    assert [ln for ln in r.stdout.decode().splitlines() if ln] == [G_COMP, G_UNC]

    o = obj(["pubkey"], SECRET1)
    assert o["type"] == "pubkey"
    assert o["encoding"] == "hex"
    assert o["network"] == "mainnet"
    assert o["compressed"] is True
    assert o["data"] == G_COMP
    assert "source" not in o

    o = obj(["pubkey"], WIF_G_COMP_TEST)
    assert o["network"] == "testnet"
    assert o["data"] == G_COMP

    o = obj(["pubkey", "--network", "testnet"], WIF_G_COMP_MAIN)
    assert o["network"] == "mainnet"

    o = obj(["pubkey", "--network", "testnet"], SECRET1)
    assert o["network"] == "testnet"
    assert o["data"] == G_COMP

    priv_obj = {
        "type": "privkey",
        "encoding": "hex",
        "network": "testnet",
        "compressed": True,
        "data": SECRET1,
    }
    o = obj(["pubkey"], json.dumps(priv_obj) + "\n")
    assert o["type"] == "pubkey"
    assert o["network"] == "testnet"
    assert o["data"] == G_COMP
    assert "source" not in o

    o = obj(["pubkey", "--source"], json.dumps(priv_obj) + "\n")
    assert o["source"]["type"] == "privkey"
    assert o["source"]["data"] == SECRET1
    assert o["source"]["network"] == "testnet"

    pub_obj = {
        "type": "pubkey",
        "encoding": "hex",
        "network": "testnet",
        "compressed": False,
        "data": G_UNC,
    }
    o = obj(["pubkey", "--compressed"], json.dumps(pub_obj) + "\n")
    assert o["data"] == G_COMP
    assert o["network"] == "testnet"
    assert "source" not in o
    o = obj(["pubkey", "--compressed", "--source"], json.dumps(pub_obj) + "\n")
    assert o["source"]["data"] == G_UNC

    pipe = (WIF_G_COMP_MAIN + "\n" + WIKI_HEX + "\n").encode()
    r = expect_ok(["pubkey"], input_bytes=pipe)
    assert [x["data"] for x in ndjson(r.stdout)] == [G_COMP, WIKI_COMP]

    r = expect_ok(["pubkey"], input_bytes=b"")
    assert r.stdout == b""

    r = expect_ok(["privkey", "--new"])
    generated = ndjson(r.stdout)[0]
    derived = obj(["pubkey"], json.dumps(generated) + "\n")
    assert derived["type"] == "pubkey"
    assert derived["network"] == generated["network"]
    assert "source" not in derived
    assert len(derived["data"]) == 66
    sourced = obj(["pubkey", "--source"], json.dumps(generated) + "\n")
    assert sourced["source"]["data"] == generated["data"]

    bare = obj(["pubkey", "--source"], WIF_G_COMP_MAIN)
    assert bare["source"]["type"] == "privkey"
    assert bare["source"]["encoding"] == "wif"
    assert bare["source"]["data"] == WIF_G_COMP_MAIN
    assert "source" not in obj(["pubkey"], WIF_G_COMP_MAIN)

    assert (
        ndjson(expect_ok(["pubkey"], SECRET1.upper()).stdout)[0]["data"] == G_COMP
    )

    r = expect_ok(["pubkey", "--out", "json"], SECRET1)
    pretty = json.loads(r.stdout.decode())
    assert pretty["data"] == G_COMP

    sixty_four_ones = "1" * 64
    hex_priv = plain(["pubkey", "--out", "plain"], sixty_four_ones)
    dec_priv = plain(["pubkey", "--from", "dec", "--out", "plain"], sixty_four_ones)
    assert hex_priv != dec_priv
    assert hex_priv == plain(["pubkey", "--from", "hex", "--out", "plain"], sixty_four_ones)

    expect_err(["pubkey"], "not a private or public key", "test01")
    expect_err(["pubkey"], "not a private or public key", "not-a-key")
    expect_err(["pubkey", "--from", "text"], "invalid --from")
    expect_err(["pubkey", "--from", "file"], "invalid --from")
    expect_err(["pubkey", "--from", "foo"], "invalid --from")
    expect_err(["pubkey", SECRET1], "provide input on stdin")
    expect_err(["pubkey", "--match", "x"], "unknown option")
    expect_err(
        ["pubkey"],
        "invalid WIF checksum",
        "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWo",
    )
    expect_err(["pubkey"], "out of range", ZERO)
    expect_err(["pubkey"], "out of range", N)
    expect_err(["pubkey"], "out of range", "0")
    expect_err(["pubkey"], "not a private or public key", '{"type":"address","data":"1BgGZ9"}\n')
    expect_err(["pubkey", "--from", "hex"], "not a private or public key", "1")
    expect_err(["pubkey"], "not a private or public key", b"\xff\xd8\xff\x00not-a-key")

    r = expect_ok(["pubkey", "--help"])
    assert r.stdout.decode() == PUBKEY_HELP

    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except AssertionError as e:
        print("assertion failed:", e, file=sys.stderr)
        sys.exit(1)
