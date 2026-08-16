# Bitcoin Toolkit 4.0.0

Git-style command-line tools for Bitcoin keys, addresses, a P2P handshake, and a local address-balance index.

This tree is a ground-up C++17 rewrite. The 3.1.2 C sources live on the `legacy/3.1.2` tag. The rebuild plan is in [REBUILD.md](REBUILD.md).

## Status

Phase 4 is implemented: `btk privkey`, `btk pubkey`, `btk address`, and `btk node`. Next is `btk balance` (`--sync` from Core JSON-RPC into `~/.btk/balance`; query addresses on stdin). Later: `config`. Help and version are `--help` / `--version` (no `help` or `version` command).

```sh
btk privkey --new
btk privkey --new --out plain
printf '%s' KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn | btk privkey --encoding hex
printf 1 | btk privkey --encoding dec --out plain
printf 1 | btk privkey --out plain
printf test01 | btk privkey --from text --out plain
cat photo.jpg | btk privkey --from file --out plain
printf '%s' KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn | btk pubkey --out plain
btk privkey --new | btk pubkey
btk privkey --new | btk pubkey --source
btk privkey --new | btk address --type p2wpkh
btk privkey --new --out plain | btk address --type p2tr --out plain
btk privkey --new --stream | btk address --type p2pkh --match '^1bri'
btk node --host seed.bitcoin.sipa.be
btk node --host seed.bitcoin.sipa.be --out plain
```

Input is stdin only. `privkey` guess order is WIF, 64-char hex, decimal, text (SHA-256), then binary (whole stream). `--from wif|hex|dec|text|file` overrides the guess — e.g. `printf 1 | btk privkey --from text` hashes the character `1` instead of treating it as secret 1. SHA-256 of a passphrase is not a KDF. Do not use it as a wallet.

`pubkey` does not hash leftover text. Guess order is WIF, 64-char hex priv, decimal, then 66/130-char hex pub. `--from` is only `wif|hex|dec`. `source` is omitted unless `--source` is set.

`address` has no `--from`. Bare lines are WIF or a 66/130-char hex public key. Typed `privkey` / `pubkey` objects still compose. `--type` is `p2pkh`, `p2wpkh` (default), or `p2tr` (BIP-341 empty-tree). `source` is included on `--match`, or when `--source` is set.

`node` is a one-shot IPv4 mainnet handshake. `--host` is required (no positional host). It sends `version`, prints the peer’s `version`, and closes without `verack`. `--out plain` prints `ip:port`.

Output is one JSON object per line (ndjson) unless `--out json` or `--out plain`.

## Build

One required library: [libsecp256k1](https://github.com/bitcoin-core/secp256k1).

```sh
# Debian / Ubuntu
sudo apt-get install libsecp256k1-dev

# Fedora
sudo dnf install libsecp256k1-devel

# macOS
brew install secp256k1

make
make test          # unit + offline CLI; no network
sudo make install  # prefix=/usr/local
```

LevelDB is optional and unused until the balance command. JSON parsing uses vendored [picojson](third_party/README.md). SHA-256 and RIPEMD-160 are implemented in-tree.

## License

GNU GPL v3. See [LICENSE](LICENSE).
