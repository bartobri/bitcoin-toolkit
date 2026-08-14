# Bitcoin Toolkit 4.0.0

Git-style command-line tools for Bitcoin keys, addresses, a P2P handshake, and a local address-balance index.

This tree is a ground-up C++17 rewrite. The 3.1.2 C sources live on the `legacy/3.1.2` tag. The rebuild plan is in [REBUILD.md](REBUILD.md).

## Status

Phase 0 (scaffold) is in progress. `bin/btk` builds and rejects unknown commands. User commands land in later phases: `privkey`, `pubkey`, `address`, `node`, `help`, `version`, `balance`, `config`.

```sh
btk --help
btk --version
btk privkey --new | btk address --type p2wpkh   # after later phases
```

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
