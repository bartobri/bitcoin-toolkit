# Bitcoin Toolkit 4.0.0

Git-style command-line tools for Bitcoin keys, addresses, a P2P handshake, and a local address-balance index.

This tree is a ground-up C++17 rewrite. The 3.1.2 C sources live on the `legacy/3.1.2` tag. The rebuild plan is in [REBUILD.md](REBUILD.md).

## Status

Phase 2 is implemented: `btk privkey` creates and converts private keys; `btk pubkey` derives or recompresses public keys. Next is `btk address`. Later commands (`node`, `help`, `version`, `balance`, `config`) land in later phases.

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
```

Input is stdin only. `privkey` guess order is WIF, 64-char hex, decimal, text (SHA-256), then binary (whole stream). `--from wif|hex|dec|text|file` overrides the guess — e.g. `printf 1 | btk privkey --from text` hashes the character `1` instead of treating it as secret 1. SHA-256 of a passphrase is not a KDF. Do not use it as a wallet.

`pubkey` does not hash leftover text. Guess order is WIF, 64-char hex priv, decimal, then 66/130-char hex pub. `--from` is only `wif|hex|dec`. `source` is omitted unless `--source` is set.

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
