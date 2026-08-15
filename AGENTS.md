# Agent notes — Bitcoin Toolkit 4.0.0

Hand-off for anyone (human or agent) continuing the 4.x rebuild. Read this first, then [REBUILD.md](REBUILD.md) for the full product spec, vectors, and later-phase algorithms.

## Where we are

Branch `4.x`. Latest work is **Phase 1 complete** (`btk privkey`). Next implementable phase is **Phase 2 (`btk pubkey`)**.

| Commit | What |
|---|---|
| `a40c142` | Wipe of 3.1.2; only `LICENSE` + `REBUILD.md` kept |
| `f9b4631` | Phase 0 scaffold |
| `021ec27` | Phase 1 privkey + later stdin-only / `--from` contract |

`make test` is the gate (unit + offline CLI). No network. Version is `4.0.0` (`src/version.hpp`).

## Sources of truth

1. **This file** — progress, lived-in CLI contract, how to resume.
2. **[REBUILD.md](REBUILD.md)** — product design, golden vectors (Appendix A), remaining phase algorithms, help appendix. **Phase 1 CLI in that file is partly stale**: it still mentions `--from-text` / `--from-file` / positionals in some tables. Prefer the contract below when they conflict.
3. **Do not** restore 3.1.2 sources, tests, man pages, cJSON, or QR. Tag `legacy/3.1.2` is history only.

## Phase status

| Phase | Status | Notes |
|---|---|---|
| 0 Scaffold | **Done** | Makefile, dispatcher, options, NDJSON I/O, hash/hex/base58, secp RAII, picojson, `--help`/`--version` stubs |
| 1 `privkey` | **Done** | See contract below. Man page `man/btk-privkey.1` |
| 2 `pubkey` | **Next** | REBUILD § Commands.2 / PR 2. Follow the **stdin-only + `--from`** contract, not 3.1.2 positionals |
| 3 `address` | Not started | bech32, bech32m, BIP-341 empty-tree p2tr, `--match` |
| 4 `node` | Not started | IPv4 mainnet handshake only; live tests behind `BTK_RUN_NET=1` |
| 5 `help` | Partial | `btk --help` / `btk privkey --help` exist. No `help` command yet. Appendix C pins overview + privkey bodies |
| 6 `version` | Stub | `btk --version` emits the typed object. No `version` command yet |
| 7a–7d `balance` | Not started | LevelDB optional; primitives then query then chainstate then RPC |
| 8 `config` | Not started | Load config **only** for `config` and `balance`. Phases 1–6 must not open `~/.btk` |

## Phase 1 contract (as shipped)

This is what `btk privkey` actually does. Carry these rules forward unless the user changes them.

### Invocation

```text
btk privkey --new [--count N] [--stream]
            [--encoding wif|hex|dec] [--network mainnet|testnet]
            [--compressed | --uncompressed]
btk privkey [--encoding wif|hex|dec] [--network mainnet|testnet]
            [--compressed | --uncompressed]
            [--from wif|hex|dec|text|file]
```

- **Stdin only.** Positional keys are an error: `provide input on stdin`. `--count 5` is still a flag argument.
- **No `--from-text` / `--from-file`.** Replaced by piping + `--from`.
- `--new` and `--from` cannot be combined.
- `--count` / `--stream` require `--new`.
- Default output is NDJSON, one typed object per line, `fflush` after each. `--out plain` prints `data`. `--out json` pretty-prints (array if N>1) unless `--stream`.

### Stdin guess (`--from` omitted, `--in auto`)

On a **pipe** (not a TTY), I/O peeks 8KiB:

1. **Binary** (NUL, C0 controls other than tab/LF/CR, or invalid UTF-8) → SHA-256 of the **entire** stdin (one key). Same as `--from file`.
2. Else `{` → incremental object stream; `[` → one JSON array; else **plain lines**.

Each **plain line** / bare string:

1. WIF (base58check, 33/34-byte payload, version `0x80`/`0xEF`)
2. 64-char hex (case-insensitive). **64-digit all-numeric is hex, not decimal.**
3. `[0-9]+` decimal (leading zeros stripped; `001` is secret 1)
4. Else SHA-256 of the line (newline already stripped)

A **WIF-shaped** string with a bad checksum is `invalid WIF checksum`, not a hash. `0` / `≥ n` is `private key out of range`, not a hash.

`--from` forces the type:

| `--from` | Behavior |
|---|---|
| `wif` / `hex` / `dec` | Parse each line as that type; error if it does not fit |
| `text` | SHA-256 each line (how to force `"1"` to be a passphrase) |
| `file` | Generator: SHA-256 entire stdin as raw bytes, one key |

`--from text|wif|hex|dec` with `--in auto` is coerced to `--in plain`. `--from file` does not read the object stream.

Typed JSON objects still work (`type=privkey`, `encoding`, `data` as a **string** — decimal `data` must not be a JSON number).

### Output encodings

`--encoding wif` (default) | `hex` | `dec`. Decimal `data` is a digit string with no leading zeros.

### Crypto / deps

- Secrets checked with `secp256k1_ec_seckey_verify`. CSPRNG: `getentropy` else `/dev/urandom`.
- SHA-256 / RIPEMD-160 are in-tree (`src/core/hash.cpp`). No OpenSSL.
- Required package: `libsecp256k1`. LevelDB probed, unused until Phase 7.
- No GMP; 256-bit decimal is a 32-byte ×10/÷10 loop in `src/core/privkey.cpp`.

### Help

`btk privkey --help` is pinned **byte-for-byte** in `test/cli/test_privkey.py` (`PRIVKEY_HELP`) and in the command’s raw string. Edit both together. Appendix C in REBUILD.md should stay in lockstep.

## Layout that exists

```
src/main.cpp
src/cli/          dispatcher, options, io, output
src/cmd/          command.hpp, privkey.{hpp,cpp}
src/core/         hash, hex, base58, json_io, secp, random, privkey, network.hpp
src/util/         error
src/version.hpp
man/btk-privkey.1
test/unit/        hash, hex, base58, privkey  (CHECK() macro, no gtest)
test/cli/         test_scaffold.py, test_privkey.py
test/runner.py    discovers test/cli/test_*.py
third_party/picojson/
```

Register new commands in `register_builtin_commands()` (`src/cli/dispatcher.cpp`). Add the `.cpp` to `SRC` in the Makefile. Unit tests are extra `bin/test_*` targets.

**Makefile has no header deps.** Changing a widely included header (especially `Options` layout) without touching every `.cpp` can produce silent ABI/stack bugs. `make clean && make test` after those edits.

## How to continue (Phase 2)

Implement `btk pubkey` from REBUILD.md §2, but **match Phase 1’s input contract**:

- Stdin only; reject positionals the same way.
- Accept typed `privkey` / `pubkey` objects and bare WIF / hex priv / hex pub via guess (and `--from` if a pubkey-side override is needed).
- From a secret: `secp256k1_ec_pubkey_create` + serialize. From a pubkey: parse + recompress.
- Default compression follows the input; `--compressed` / `--uncompressed` override; both flags → two objects.
- Output `encoding` is always `hex`. Copy `network` from the typed input or WIF version, else `--network`, else mainnet.
- Optional `source` (the input privkey object) when the input was a privkey — address must not walk `source` for network (one level only). `--match` is an unknown flag here.
- Acceptance: Appendix A Vector G and Wiki compressed/uncompressed hex; WIF → pubkey; recompress; testnet privkey object → pubkey object with `network=testnet`.
- New files: `src/cmd/pubkey.cpp`, `src/core/pubkey.{hpp,cpp}`, `test/cli/test_pubkey.py`, unit coverage, `man/btk-pubkey.1`.
- `btk pubkey --help` from Appendix C (adapt if the stdin-only wording should match privkey).

After Phase 2, Phase 3 is `address` (bech32 + BIP-341). Do not skip goldens A.2 / A.2b / A.6.

Phases 1–6 must not load `~/.btk/config.json`.

## Conventions

- C++17, GNU Makefile, no Boost, no CMake. Unix only.
- Errors: `BtkError(command, message)` → `btk <command>: <message>` on stderr, exit 1. Never put WIF/hex/passphrases in messages.
- Exceptions internally; `main` maps them to exit 1.
- Tests compare parsed JSON objects, not string tables. CLI tests spawn `bin/btk`.
- `src/cli/io.cpp` owns stdin framing (`--in auto|ndjson|json|plain`) and binary-prefix detection. Commands must not assume positionals.

## Quick commands

```sh
make
make test          # unit + offline CLI
make test-unit
make test-cli
make clean && make test   # after header/layout changes
```
