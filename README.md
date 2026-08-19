# Bitcoin Toolkit

Git-style command-line tools for Bitcoin.

`btk` is a single Unix binary. Commands take keys and addresses on stdin,
print typed JSON on stdout, and compose with pipes — the same way `git`
composes subcommands and the same way `jq` composes filters.

```sh
btk privkey --new | btk address --type p2tr
```

```json
{"type":"address","style":"p2tr","network":"mainnet","data":"bc1p…"}
```

Create keys. Convert encodings. Derive public keys. Make P2PKH, P2WPKH, and
real BIP-341 Taproot addresses. Handshake a P2P peer. Keep a local
address→satoshi index fed by Bitcoin Core RPC.

This is a toolkit, not a wallet. It will not store funds, sign transactions,
or prompt you before printing a private key.

## Try it

```sh
# A fresh key and its default SegWit address
btk privkey --new | btk address

# Just the strings
btk privkey --new --out plain
btk privkey --new --out plain | btk address --type p2tr --out plain

# Hunt a vanity P2PKH. --match keeps the matching key under source.
btk privkey --new --stream | btk address --type p2pkh --match '^1bri'

# The generator point (secret 1) as hex, as a pubkey, as Taproot
printf '%s' KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn \
  | btk privkey --encoding hex --out plain
# 0000000000000000000000000000000000000000000000000000000000000001

printf '%s' KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn \
  | btk pubkey --out plain
# 0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798

printf '%s' KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn \
  | btk address --type p2tr --out plain
# bc1pmfr3p9j00pfxjh0zmgp99y8zftmd3s5pmedqhyptwy6lm87hf5sspknck9

# Shake hands with a seed node. --out plain prints ip:port.
btk node --host seed.bitcoin.sipa.be
btk node --host seed.bitcoin.sipa.be --out plain

# Index the chain (needs a local bitcoind) and look up an address
btk config set rpc.host=127.0.0.1
btk config set rpc.port=8332
btk config set rpc.auth=user:pass
btk balance --sync
printf '%s' 1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa | btk balance
```

## How it works

Each command is `btk <command> [options]`. Payloads — keys, addresses — travel
on **stdin**, never as leftover argv. `printf KEY | btk pubkey` is right;
`btk pubkey KEY` is `provide input on stdin`.

Default output is **NDJSON**: one typed JSON object per line, flushed as it is
produced, so a producer that never exits (vanity search) still drives a
consumer. `--out json` pretty-prints. `--out plain` prints the primary string
only (WIF, hex, address, satoshis, `ip:port`).

A typed object always wins over a bare line. Pipe `btk privkey --new` into
`btk address` and the consumer sees `type=privkey`. Pipe a WIF string and it
is determined. `--from` overrides that determination where it would be
ambiguous (`privkey`, `pubkey`). `--in` is framing (`auto` / `ndjson` /
`json` / `plain`), not meaning.

`--source` records where an item came from so a later stage can still see the
key. `--match` on `address` implies it — that is how vanity search returns a
spendable secret next to the matching address.

```sh
btk privkey --new | btk address --source | btk balance --source
```

## Commands

| Command | Job |
|---|---|
| `btk privkey` | Create a CSPRNG key (`--new`), convert WIF / hex / decimal, or SHA-256 a passphrase / file |
| `btk pubkey` | Derive or recompress a public key from an explicit private or public key |
| `btk address` | Derive `p2pkh`, `p2wpkh` (default), or BIP-341 `p2tr` |
| `btk node` | IPv4 mainnet `version` handshake (`--host` required) |
| `btk balance` | Query `~/.btk/balance`, or `--sync` it from Core JSON-RPC |
| `btk config` | `set` / `get` / `unset` / `dump` RPC defaults |

There is no `help` or `version` command. Use `btk --help`, `btk <command> --help`,
and `btk --version`.

`--network mainnet|testnet` applies to keys and addresses. Network is per
object (a WIF version byte wins over the flag). `btk node` is IPv4 mainnet
only.

## Private keys

`--new` draws 32 bytes from `getentropy` (else `/dev/urandom`) and rejects
scalars outside `[1, n-1]`. Default encoding is compressed mainnet WIF.

Without `--new`, each stdin line is tried as WIF, then 64-character hex, then
decimal; leftover text is SHA-256 of the line. Piped binary is SHA-256 of the
whole stream. `--from wif|hex|dec|text|file` forces the type — that is how
`printf 1 | btk privkey --from text` hashes the character `1` instead of
taking secret 1.

SHA-256 of a passphrase is **not** a KDF. Do not use it as a wallet.

`pubkey` and `address` never invent a secret from leftover text. Hash first
with `btk privkey --from text`, then pipe the typed object.

## Balance

`btk balance --sync` walks Bitcoin Core JSON-RPC (`getblock` raw hex) into a
LevelDB index at `~/.btk/balance`. The first run starts at genesis; later runs
catch up from the last saved height. Ctrl-C aborts within ~200 ms; the next
`--sync` continues. A reorg refuses to proceed until you pass `--sync --force`.

Query is stdin-only. A missing address is `sats: 0`. `--skip-zero` omits those.
`--out plain` prints the satoshi count.

```sh
printf '%s' 1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa | btk balance --out plain
```

RPC defaults live in `~/.btk/config.json` (`rpc.host`, `rpc.port`, `rpc.auth`).
`btk config dump` always redacts the password as eight asterisks. Only
`config` and `balance` open that file.

## Build

Unix only. One required library: [libsecp256k1](https://github.com/bitcoin-core/secp256k1).
LevelDB is optional and needed for `btk balance`. SHA-256 and RIPEMD-160 are
in-tree. JSON is vendored [picojson](third_party/README.md).

```sh
# Debian / Ubuntu
sudo apt-get install libsecp256k1-dev libleveldb-dev

# Fedora
sudo dnf install libsecp256k1-devel leveldb-devel

# macOS
brew install secp256k1 leveldb

make
make test          # unit + offline CLI; no network
sudo make install  # prefix=/usr/local
```

`make test-net` is a live P2P handshake (`BTK_RUN_NET=1`). Default `make test`
never touches the network.

```sh
btk --version --out plain
# 4.0.0
```

## License

GNU GPL v3. See [LICENSE](LICENSE).
