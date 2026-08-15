#!/usr/bin/env python3
"""Phase 3 btk address CLI tests."""

import json
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
BTK = ROOT / "bin" / "btk"

SECRET1 = "0000000000000000000000000000000000000000000000000000000000000001"
SECRET6 = "0000000000000000000000000000000000000000000000000000000000000006"
WIKI_HEX = "18e14a7b6a307f426a94f8114701e7c8e774e7f9a47e2c2035db29a206321725"
A6_HEX = "6b973d88838f27366ed61c9ad6367663045cb456e28335c109e30717ae0c6baa"
G_XONLY = "79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798"

WIF_G_COMP_MAIN = "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn"
WIF_G_UNC_MAIN = "5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreAnchuDf"
WIF_G_COMP_TEST = "cMahea7zqjxrtgAbB7LSGbcQUr1uX1ojuat9jZodMN87JcbXMTcA"
WIF_WIKI_UNC = "5J1F7GHadZG3sCCKHCwg8Jvys9xUbFsjLnGec4H125Ny1V9nR6V"
WIF_SEC6 = "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU76Myig6zj"

G_COMP = "0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798"
G_UNC = (
    "0479be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798"
    "483ada7726a3c4655da4fbfc0e1108a8fd17b448a68554199c47d08ffb10d4b8"
)
SEC6_COMP = "03fff97bd5755eeea420453a14355235d382f6472f8568a18b2f057a1460297556"
SEC6_EVEN = "02fff97bd5755eeea420453a14355235d382f6472f8568a18b2f057a1460297556"

P2PKH_G = "1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH"
P2PKH_G_UNC = "1EHNa6Q4Jz2uvNExL497mE43ikXhwF6kZm"
P2PKH_G_TEST = "mrCDrCybB6J1vRfbwM5hemdJz73FwDBC8r"
P2WPKH_G = "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4"
P2WPKH_G_TEST = "tb1qw508d6qejxtdg4y5r3zarvary0c5xw7kxpjzsx"
P2TR_G = "bc1pmfr3p9j00pfxjh0zmgp99y8zftmd3s5pmedqhyptwy6lm87hf5sspknck9"
P2TR_G_TEST = "tb1pmfr3p9j00pfxjh0zmgp99y8zftmd3s5pmedqhyptwy6lm87hf5ssk79hv2"
P2TR_UNTWEAKED_G = "bc1p0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7vqzk5jj0"

P2PKH_WIKI_UNC = "16UwLL9Risc3QfPqBUvKofHmBQ7wMtjvM"
P2PKH_WIKI = "1PMycacnJaSqwwJqjawXBErnLsZ7RkXUAs"
P2WPKH_WIKI = "bc1q7499s50fxu4c0qg23esvm5h8elvqkm33r2tdza"
P2TR_WIKI = "bc1p5ymjdxmqesnfeg42lyjh642570nxp7dgp0uzh22uq4z3gew9yymst6pshk"

P2PKH_SEC6 = "1Cf2hs39Woi61YNkYGUAcohL2K2q4pawBq"
P2WPKH_SEC6 = "bc1q0ldfeupqc9k2eaffep7cm6yml3ct3jwtwzqt7k"
P2TR_SEC6 = "bc1p4rsld9ryjhte00drc0r23r8ngd63xrzh5s4fvmy6q5yt70xzlsdqcuvtzv"

P2TR_A6 = "bc1p2wsldez5mud2yam29q22wgfh9439spgduvct83k3pm50fcxa5dps59h4z5"

ADDRESS_HELP = """btk address — derive addresses

Usage:
  btk address [--type p2pkh|p2wpkh|p2tr]...
              [--match REGEX] [--ignore-case]
              [--network mainnet|testnet]
              [--source]

  --type       Repeatable. Default: p2wpkh
               p2pkh   Base58Check HASH160(pubkey)
               p2wpkh  Bech32 v0 HASH160(compressed pubkey)
               p2tr    Bech32m v1 BIP-341 key-path (empty tree)
  --match      POSIX ERE on the address; drop non-matches; includes source
  --source     Include a source object even without --match

Items are a privkey or pubkey object, or a stdin line that is
already a WIF private key or a 66/130-char hex public key.
Guess order: WIF, then hex pub. There is no --from.
64-hex and leftover text are errors (not keys).
p2wpkh and p2tr require a compressed key.
There is no --bech32m flag; use --type p2tr for Taproot.

Vanity:
  btk privkey --new --stream | btk address --type p2pkh --match '^1bri'
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
    assert "btk address:" in err, err
    return r


def plain(args, data):
    r = expect_ok(args, input_bytes=data)
    return r.stdout.decode().strip()


def obj(args, data):
    return ndjson(expect_ok(args, input_bytes=data).stdout)[0]


def priv_obj(data, encoding="hex", network="mainnet", compressed=True):
    return json.dumps(
        {
            "type": "privkey",
            "encoding": encoding,
            "network": network,
            "compressed": compressed,
            "data": data,
        }
    ) + "\n"


def pub_obj(data, network="mainnet", compressed=True):
    return json.dumps(
        {
            "type": "pubkey",
            "encoding": "hex",
            "network": network,
            "compressed": compressed,
            "data": data,
        }
    ) + "\n"


def main():
    assert plain(["address", "--out", "plain"], WIF_G_COMP_MAIN) == P2WPKH_G
    assert plain(["address", "--type", "p2wpkh", "--out", "plain"], WIF_G_COMP_MAIN) == P2WPKH_G
    assert plain(["address", "--type", "p2pkh", "--out", "plain"], WIF_G_COMP_MAIN) == P2PKH_G
    assert plain(["address", "--type", "p2tr", "--out", "plain"], WIF_G_COMP_MAIN) == P2TR_G
    assert P2TR_G != P2TR_UNTWEAKED_G
    assert len(P2TR_G) == 62

    assert plain(["address", "--out", "plain"], G_COMP) == P2WPKH_G
    assert plain(["address", "--type", "p2pkh", "--out", "plain"], G_COMP) == P2PKH_G
    assert plain(["address", "--type", "p2tr", "--out", "plain"], G_COMP) == P2TR_G
    assert plain(["address", "--type", "p2pkh", "--out", "plain"], G_UNC) == P2PKH_G_UNC
    assert plain(["address", "--type", "p2pkh", "--out", "plain"], WIF_G_UNC_MAIN) == P2PKH_G_UNC

    assert plain(["address", "--type", "p2wpkh", "--out", "plain"], WIF_G_COMP_TEST) == P2WPKH_G_TEST
    assert plain(["address", "--type", "p2pkh", "--out", "plain"], WIF_G_COMP_TEST) == P2PKH_G_TEST
    assert plain(["address", "--type", "p2tr", "--out", "plain"], WIF_G_COMP_TEST) == P2TR_G_TEST

    assert plain(["address", "--type", "p2pkh", "--out", "plain"], WIF_WIKI_UNC) == P2PKH_WIKI_UNC
    assert (
        plain(["address", "--type", "p2pkh", "--out", "plain"], priv_obj(WIKI_HEX, compressed=True))
        == P2PKH_WIKI
    )
    assert plain(["address", "--out", "plain"], priv_obj(WIKI_HEX)) == P2WPKH_WIKI
    assert plain(["address", "--type", "p2tr", "--out", "plain"], priv_obj(WIKI_HEX)) == P2TR_WIKI

    assert plain(["address", "--type", "p2pkh", "--out", "plain"], WIF_SEC6) == P2PKH_SEC6
    assert plain(["address", "--out", "plain"], WIF_SEC6) == P2WPKH_SEC6
    assert plain(["address", "--type", "p2tr", "--out", "plain"], WIF_SEC6) == P2TR_SEC6
    assert plain(["address", "--type", "p2tr", "--out", "plain"], SEC6_COMP) == P2TR_SEC6
    assert plain(["address", "--type", "p2tr", "--out", "plain"], SEC6_EVEN) == P2TR_SEC6

    assert plain(["address", "--type", "p2tr", "--out", "plain"], priv_obj(A6_HEX)) == P2TR_A6
    r = expect_ok(["privkey", "--from", "hex", "--encoding", "wif"], A6_HEX)
    wif_obj = ndjson(r.stdout)[0]
    assert obj(["address", "--type", "p2tr"], json.dumps(wif_obj) + "\n")["data"] == P2TR_A6

    r = expect_ok(
        ["address", "--type", "p2pkh", "--type", "p2wpkh", "--type", "p2tr", "--out", "plain"],
        WIF_G_COMP_MAIN,
    )
    assert [ln for ln in r.stdout.decode().splitlines() if ln] == [P2PKH_G, P2WPKH_G, P2TR_G]

    o = obj(["address"], WIF_G_COMP_MAIN)
    assert o["type"] == "address"
    assert o["style"] == "p2wpkh"
    assert o["network"] == "mainnet"
    assert o["data"] == P2WPKH_G
    assert "source" not in o

    o = obj(["address", "--source"], WIF_G_COMP_MAIN)
    assert o["source"]["type"] == "privkey"
    assert o["source"]["encoding"] == "wif"
    assert o["source"]["data"] == WIF_G_COMP_MAIN
    assert o["source"]["compressed"] is True

    o = obj(["address", "--source"], G_COMP)
    assert o["source"]["type"] == "pubkey"
    assert o["source"]["encoding"] == "hex"
    assert o["source"]["data"] == G_COMP

    typed = priv_obj(SECRET1)
    o = obj(["address"], typed)
    assert o["data"] == P2WPKH_G
    assert "source" not in o
    o = obj(["address", "--source"], typed)
    assert o["source"]["type"] == "privkey"
    assert o["source"]["data"] == SECRET1

    o = obj(["address"], pub_obj(G_COMP, network="testnet"))
    assert o["network"] == "testnet"
    assert o["data"] == P2WPKH_G_TEST
    assert "source" not in o
    o = obj(["address", "--source"], pub_obj(G_COMP, network="testnet"))
    assert o["source"]["type"] == "pubkey"

    o = obj(["address", "--network", "testnet"], G_COMP)
    assert o["network"] == "testnet"
    assert o["data"] == P2WPKH_G_TEST

    o = obj(["address", "--network", "testnet"], WIF_G_COMP_MAIN)
    assert o["network"] == "mainnet"
    assert o["data"] == P2WPKH_G

    o = obj(["address"], priv_obj(SECRET1, network="testnet"))
    assert o["network"] == "testnet"
    assert o["data"] == P2WPKH_G_TEST

    wif_main_obj = {
        "type": "privkey",
        "encoding": "wif",
        "network": "testnet",
        "compressed": True,
        "data": WIF_G_COMP_MAIN,
    }
    o = obj(["address"], json.dumps(wif_main_obj) + "\n")
    assert o["network"] == "mainnet"

    r = expect_ok(["privkey", "--new"])
    generated = ndjson(r.stdout)[0]
    derived = obj(["address"], json.dumps(generated) + "\n")
    assert derived["type"] == "address"
    assert derived["style"] == "p2wpkh"
    assert derived["network"] == generated["network"]
    assert "source" not in derived
    sourced = obj(["address", "--source"], json.dumps(generated) + "\n")
    assert sourced["source"]["data"] == generated["data"]
    via_pub = obj(["pubkey"], json.dumps(generated) + "\n")
    from_pub = obj(["address", "--source"], json.dumps(via_pub) + "\n")
    assert from_pub["data"] == derived["data"]
    assert from_pub["source"]["type"] == "pubkey"

    r = expect_ok(["address", "--type", "p2pkh", "--match", "^1BgGZ9"], WIF_G_COMP_MAIN)
    matched = ndjson(r.stdout)[0]
    assert matched["data"] == P2PKH_G
    assert matched["source"]["type"] == "privkey"
    assert matched["source"]["data"] == WIF_G_COMP_MAIN
    r = expect_ok(["address", "--type", "p2pkh", "--match", "^1bri"], WIF_G_COMP_MAIN)
    assert r.stdout == b""
    r = expect_ok(["address", "--type", "p2pkh", "--match", "^1BG"], WIF_G_COMP_MAIN)
    assert r.stdout == b""
    r = expect_ok(
        ["address", "--type", "p2pkh", "--match", "^1BG", "--ignore-case"], WIF_G_COMP_MAIN
    )
    icase = ndjson(r.stdout)[0]
    assert icase["data"] == P2PKH_G
    assert icase["source"]["data"] == WIF_G_COMP_MAIN
    typed_match = obj(["address", "--type", "p2pkh", "--match", "^1BgGZ9"], priv_obj(SECRET1))
    assert typed_match["data"] == P2PKH_G
    assert typed_match["source"]["type"] == "privkey"
    assert typed_match["source"]["data"] == SECRET1

    pipe = (WIF_G_COMP_MAIN + "\n" + WIF_SEC6 + "\n").encode()
    r = expect_ok(["address", "--type", "p2pkh"], input_bytes=pipe)
    assert [x["data"] for x in ndjson(r.stdout)] == [P2PKH_G, P2PKH_SEC6]

    r = expect_ok(["address"], input_bytes=b"")
    assert r.stdout == b""

    r = expect_ok(["address", "--out", "json"], WIF_G_COMP_MAIN)
    pretty = json.loads(r.stdout.decode())
    assert pretty["data"] == P2WPKH_G

    expect_err(["address"], "not a private or public key", SECRET1)
    expect_err(["address"], "not a private or public key", G_XONLY)
    expect_err(["address"], "not a private or public key", "1")
    expect_err(["address"], "not a private or public key", "test01")
    expect_err(["address"], "not a private or public key", "not-a-key")
    expect_err(["address"], "not a private or public key", b"\xff\xd8\xff\x00not-a-key")
    expect_err(["address"], "not a private or public key", '{"type":"address","data":"1BgGZ9"}\n')
    expect_err(["address", "--no-source"], "unknown option")
    expect_err(["address", "--from", "wif"], "unknown option")
    expect_err(["address", "--from", "text"], "unknown option")
    expect_err(["address", WIF_G_COMP_MAIN], "provide input on stdin")
    expect_err(["address", "--count", "2"], "unknown option '--count'", WIF_G_COMP_MAIN)
    expect_err(["address", "--type", "bech32m"], "unknown address type", WIF_G_COMP_MAIN)
    expect_err(
        ["address", "--type", "p2wpkh"],
        "uncompressed key cannot produce p2wpkh or p2tr",
        WIF_G_UNC_MAIN,
    )
    expect_err(
        ["address", "--type", "p2tr"],
        "uncompressed key cannot produce p2wpkh or p2tr",
        G_UNC,
    )
    expect_err(
        ["address"],
        "invalid WIF checksum",
        "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWo",
    )
    expect_err(["address", "--match", "["], "invalid match pattern", WIF_G_COMP_MAIN)
    expect_err(
        ["address", "--match", "a", "--match", "b"],
        "cannot pass --match more than once",
        WIF_G_COMP_MAIN,
    )

    r = expect_ok(["address", "--help"])
    assert r.stdout.decode() == ADDRESS_HELP

    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except AssertionError as e:
        print("assertion failed:", e, file=sys.stderr)
        sys.exit(1)
