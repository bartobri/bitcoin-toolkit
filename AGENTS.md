# Agent notes — Bitcoin Toolkit 4.0.0

Hand-off for anyone (human or agent) continuing the 4.x rebuild. Read this first, then [REBUILD.md](REBUILD.md) for the full product spec, vectors, and later-phase algorithms.

## Where we are

Branch `4.x`. Latest work is **Phase 2 complete** (`btk pubkey`). Next implementable phase is **Phase 3 (`btk address`)**.

| Commit | What |
|---|---|
| `a40c142` | Wipe of 3.1.2; only `LICENSE` + `REBUILD.md` kept |
| `f9b4631` | Phase 0 scaffold |
| `021ec27` | Phase 1 privkey + later stdin-only / `--from` contract |
| `e5281ad` | Pubkey plan: no leftover hash; guess WIF → hex priv → dec → hex pub |
| `38855af` | Phase 2 `btk pubkey` + opt-in `--source` |

`make test` is the gate (unit + offline CLI). No network. Version is `4.0.0` (`src/version.hpp`).

## Sources of truth

1. **This file** — progress, lived-in CLI contract, how to resume.
2. **[REBUILD.md](REBUILD.md)** — product design, golden vectors (Appendix A), remaining phase algorithms, help appendix. The **shared input contract** (stdin-only items, `--from`, no silent hash on `address`) lives there and here; keep them in lockstep.
3. **Do not** restore 3.1.2 sources, tests, man pages, cJSON, or QR. Tag `legacy/3.1.2` is history only.

## Phase status

| Phase | Status | Notes |
|---|---|---|
| 0 Scaffold | **Done** | Makefile, dispatcher, options, NDJSON I/O, hash/hex/base58, secp RAII, picojson, `--help`/`--version` stubs |
| 1 `privkey` | **Done** | See contract below. Man page `man/btk-privkey.1` |
| 2 `pubkey` | **Done** | See contract below. Man page `man/btk-pubkey.1` |
| 3 `address` | **Next** | bech32, bech32m, BIP-341 empty-tree p2tr, `--match` |
| 4 `node` | Not started | IPv4 mainnet handshake only; `--host` required (no positional host). Live tests behind `BTK_RUN_NET=1` |
| 5 `help` | Partial | `btk --help` / `btk privkey --help` / `btk pubkey --help` exist. No `help` command yet. Appendix C pins overview + privkey/pubkey bodies |
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
src/cmd/          command.hpp, privkey.{hpp,cpp}, pubkey.{hpp,cpp}
src/core/         hash, hex, base58, json_io, secp, random, privkey, pubkey, network.hpp
src/util/         error
src/version.hpp
man/btk-privkey.1
man/btk-pubkey.1
test/unit/        hash, hex, base58, privkey, pubkey  (CHECK() macro, no gtest)
test/cli/         test_scaffold.py, test_privkey.py, test_pubkey.py
test/runner.py    discovers test/cli/test_*.py
third_party/picojson/
```

Register new commands in `register_builtin_commands()` (`src/cli/dispatcher.cpp`). Add the `.cpp` to `SRC` in the Makefile. Unit tests are extra `bin/test_*` targets.

**Makefile has no header deps.** Changing a widely included header (especially `Options` layout) without touching every `.cpp` can produce silent ABI/stack bugs. `make clean && make test` after those edits.

## Shared CLI contract (later phases)

Phase 1 dropped positionals and `--from-text`/`--from-file` in favor of **stdin items + `--from` to override a guess**. Apply that only where it is the same *kind* of input (a stream of keys/addresses). Do not force it onto hosts, paths, or verbs.

Split every command into:

| Kind | What argv may contain | Input stream? |
|---|---|---|
| **Transformer** | flags only | yes — items on stdin |
| **Generator** | flags only (`--new`, `--build`, …) | no (or raw stdin if `--from file`) |
| **Parameterized one-shot** | flags for host/path/port | no |
| **Verb / topic** | subcommand + key names | no |

### Rules that apply everywhere they make sense

1. **Item payload is never a positional.** `btk pubkey <wif>` and `btk address 00…01` are errors (`provide input on stdin`). Flag arguments (`--count 5`, `--type p2tr`, `--host x`) stay.
2. **`--in` is framing** (auto / ndjson / json / plain). **`--from` is meaning** (wif / hex / dec / text / file / …). Do not merge them. `--encoding` remains **output** encoding only (`privkey`).
3. **Typed objects in the pipe always win.** `{` / `[` under `--in auto` is the object stream. `--from` applies to **bare lines**, not to an object’s `type`/`data`.
4. **`--from TYPE` is the override.** Default is guess. Unknown `--from` is `invalid --from`. Cannot combine with generators like `--new` / `--build`.
5. **Silent SHA-256 is not universal.** Only `privkey` hashes leftover text without `--from`. `pubkey` requires an explicit key. A WIF-shaped bad checksum is never hashed.
6. **`--out` / `--in` / `--network` / `--compressed` stay the global vocabulary.** Per-command flags (`--type`, `--match`, `--host`, `--build`) stay per-command.

### Per-command

| Command | Kind | Stdin-only items? | `--from` values | Guess (bare line) | Silent SHA-256? |
|---|---|---|---|---|---|
| `privkey` | transformer / `--new` generator | yes (shipped) | `wif\|hex\|dec\|text\|file` | WIF → 64-hex → dec → text | yes (after guess) |
| `pubkey` | transformer | **yes** | `wif\|hex\|dec` (`hex` = 64-char priv or 66/130-char pub) | WIF → 64-hex priv → dec → 66/130 hex pub | **no**. Leftover text / `--from text\|file` are errors |
| `address` | transformer | **yes** | `wif\|hex\|dec\|text\|file` (`hex` = **secret**; 66/130 = pub via guess or `--from` if we add `pubkey`) | WIF → 66/130 pub → 64-hex **secret** → dec | **no**. Unknown string errors. `--from text\|file` hashes to a secret then addresses. Never treat 64-hex as x-only |
| `node` | parameterized one-shot | no | none | n/a | no. `--host HOST` required. Not a key pipe |
| `help` | topic | n/a | none | n/a | no. `btk help privkey` stays a topic token |
| `version` | generator | n/a | none | n/a | no. Ignores stdin |
| `balance` query | transformer | **yes** | optional `address` | Base58Check / bech32 address only | **no** (unknown → error, not a hash). `--from-rpc` / `--from-chainstate` are **build sources**, distinct flags, not `--from` |
| `balance --build/--update` | parameterized generator | no | none | n/a | no. `--path`, `--host`, `--chainstate` stay flags |
| `config` | verb | n/a | none | n/a | no. `set`/`get`/`unset`/`dump` keep argv keys |

### Why pubkey does not silently hash

`pubkey` turns a key into a public key. It does not invent a secret from leftover text or a file. `printf test01 | btk pubkey` is `not a private or public key`. Hash a passphrase first with `btk privkey --from text`, then pipe the typed object.

### Why address does not silently hash

`printf 1BgGZ9… | btk address` looking like “use this address” must not SHA-256 the string into a new key. Privkey’s job includes “string → secret”; address’s job is “key → script”. Override is explicit: `--from text`.

### Why node/config keep non-pipe argv

`seed.bitcoin.sipa.be` and `rpc.host=127.0.0.1` are parameters, not a stream of objects. Continuity means “no naked *payloads*”, not “no words after the command.” `node` takes `--host` (D23). `config` keeps `set`/`get` argv keys.

### Naming clash

`--from wif` (privkey/pubkey/address) and `--from-rpc` (balance build) are different options. Do not invent `--from rpc`. Keep `--from-rpc` / `--from-chainstate`.

## Phase 2 contract (as shipped)

This is what `btk pubkey` actually does.

```text
btk pubkey [--compressed | --uncompressed]
           [--from wif|hex|dec] [--source]
```

- **Stdin only.** Positional keys are `provide input on stdin`.
- Accept typed `privkey` / `pubkey` objects and bare WIF / 64-hex priv / decimal / 66/130-hex pub. No leftover-text or file hash. No `--from text` / `--from file`.
- `--from` is only `wif|hex|dec`. `--from hex` is 64-char priv or 66/130-char pub. Unknown `--from` is `invalid --from`.
- Guess: WIF → 64-hex priv → decimal → 66/130 hex pub. `--from` overrides. 64-digit all-numeric is hex priv. A 66- or 130-digit all-numeric string is decimal (before hex pub). Fail: `not a private or public key`.
- From a private key: `secp256k1_ec_pubkey_create` + serialize. From a pubkey: parse + recompress.
- Default compression follows the input; `--compressed` / `--uncompressed` override; both flags → two objects.
- Output `encoding` is always `hex`. `network` from the typed input or WIF version, else `--network`, else mainnet. `--network` does **not** override a WIF version byte.
- `source` is omitted unless `--source` is set (typed parent object, or a synthesized object for a bare string). `--match` and `--no-source` are unknown flags.
- `btk pubkey --help` is pinned in `test/cli/test_pubkey.py` (`PUBKEY_HELP`) and the command’s raw string.

## How to continue (Phase 3)

Implement `btk address` from REBUILD.md §3 and the Shared input contract:

- Stdin only; reject positionals (`provide input on stdin`).
- `--type p2pkh|p2wpkh|p2tr` (repeatable; default one `p2wpkh`). `--match` is address-only.
- No silent leftover-text hash. Unknown string: `not a private or public key`. `--from text|file` hashes to a secret then addresses.
- 64-hex is always a secret, never an x-only key. Do not skip goldens A.2 / A.2b / A.6.
- Uncompressed + `p2wpkh`/`p2tr`: `uncompressed key cannot produce p2wpkh or p2tr`.
- New files: `src/cmd/address.cpp`, bech32/bech32m + BIP-341 tweak, `test/cli/test_address.py`, unit coverage, `man/btk-address.1`.

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
