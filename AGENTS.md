# Agent notes — Bitcoin Toolkit 4.0.0

Hand-off for anyone (human or agent) continuing the 4.x rebuild. Read this first, then [REBUILD.md](REBUILD.md) for the full product spec, vectors, and later-phase algorithms.

## Where we are

Branch `4.x`. Latest work is **Phase 6 complete** (`btk config`). The 4.0.0 command set is shipped. There is no `help` or `version` command; use `--help` / `--version`.

| Commit | What |
|---|---|
| `a40c142` | Wipe of 3.1.2; only `LICENSE` + `REBUILD.md` kept |
| `f9b4631` | Phase 0 scaffold |
| `021ec27` | Phase 1 privkey + later stdin-only / `--from` contract |
| `e5281ad` | Pubkey plan: no leftover hash; guess WIF → hex priv → dec → hex pub |
| `38855af` | Phase 2 `btk pubkey` + opt-in `--source` |
| `3fb1427` | Phase 3 `btk address` + `--match` implies `source` |
| `76bb1d9` | Record Phase 3 as shipped |
| `d91fa5b` | Phase 4 `btk node` IPv4 mainnet handshake |
| `385ff08` | Plan balance as `--sync` over RPC into `~/.btk/balance` |
| `fb4604f` | Phase 5 `btk balance` RPC `--sync` + LevelDB index |
| `f7e9c01` | Phase 6 `btk config` set/get/unset/dump |

`make test` is the gate (unit + offline CLI). No network. Version is `4.0.0` (`src/version.hpp`).

## Sources of truth

1. **This file** — progress, lived-in CLI contract, how to resume.
2. **[REBUILD.md](REBUILD.md)** — product design, golden vectors (Appendix A), remaining phase algorithms, help appendix. The **shared input contract** (stdin-only items, `--from` on privkey/pubkey, no `--from` / no silent hash on `address`) lives there and here; keep them in lockstep.
3. **Do not** restore 3.1.2 sources, tests, man pages, cJSON, or QR. Tag `legacy/3.1.2` is history only.

## Phase status

| Phase | Status | Notes |
|---|---|---|
| 0 Scaffold | **Done** | Makefile, dispatcher, options, NDJSON I/O, hash/hex/base58, secp RAII, picojson, `--help` / `--version` |
| 1 `privkey` | **Done** | See contract below. Man page `man/btk-privkey.1` |
| 2 `pubkey` | **Done** | See contract below. Man page `man/btk-pubkey.1` |
| 3 `address` | **Done** | See contract below. Man page `man/btk-address.1` |
| 4 `node` | **Done** | See contract below. Man page `man/btk-node.1` |
| 5a–5c `balance` | **Done** | See contract below. Man page `man/btk-balance.1`. LevelDB optional; RPC `--sync` only |
| 6 `config` | **Done** | See contract below. No man page. Load config **only** for `config` and `balance`. Phases 1–4 must not open `~/.btk` |

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
- Required package: `libsecp256k1`. LevelDB optional (`btk balance`).
- No GMP; 256-bit decimal is a 32-byte ×10/÷10 loop in `src/core/privkey.cpp`.

### Help

`btk privkey --help` is pinned **byte-for-byte** in `test/cli/test_privkey.py` (`PRIVKEY_HELP`) and in the command’s raw string. Edit both together. Appendix C in REBUILD.md should stay in lockstep.

## Layout that exists

```
src/main.cpp
src/cli/          dispatcher, options, io, output
src/cmd/          command.hpp, privkey, pubkey, address, node, balance, config
src/core/         hash, hex, base58, bech32, json_io, secp, random, privkey, pubkey, address, network.hpp
src/chain/        compactsize, transaction, script, balance_db, indexer
src/net/          p2p, jsonrpc
src/util/         error, config (load + save)
src/version.hpp
man/btk-privkey.1
man/btk-pubkey.1
man/btk-address.1
man/btk-node.1
man/btk-balance.1
test/unit/        hash, hex, base58, privkey, pubkey, bech32, address, p2p, compactsize, tx, balance_db
test/cli/         test_scaffold.py, test_privkey.py, test_pubkey.py, test_address.py, test_node.py, test_balance.py, test_config.py
test/runner.py    discovers test/cli/test_*.py
third_party/picojson/
```

Register new commands in `register_builtin_commands()` (`src/cli/dispatcher.cpp`). Add the `.cpp` to `SRC` in the Makefile. Unit tests are extra `bin/test_*` targets.

**Makefile has no header deps.** Changing a widely included header (especially `Options` layout) without touching every `.cpp` can produce silent ABI/stack bugs. `make clean && make test` after those edits.

## Shared CLI contract (later phases)

Phase 1 dropped positionals and `--from-text`/`--from-file` in favor of **stdin items + `--from` to override a guess**. Apply that only where it is the same *kind* of input (a stream of keys) **and** the bare-line guess is ambiguous. Do not force it onto hosts, paths, verbs, or `address` (WIF vs hex pub is unambiguous).

Split every command into:

| Kind | What argv may contain | Input stream? |
|---|---|---|
| **Transformer** | flags only | yes — items on stdin |
| **Generator** | flags only (`--new`, `--sync`, …) | no (or raw stdin if `--from file`) |
| **Parameterized one-shot** | flags for host/port | no |
| **Verb** | subcommand + key names | no |

### Rules that apply everywhere they make sense

1. **Item payload is never a positional.** `btk pubkey <wif>` and `btk address 00…01` are errors (`provide input on stdin`). Flag arguments (`--count 5`, `--type p2tr`, `--host x`) stay.
2. **`--in` is framing** (auto / ndjson / json / plain). **`--from` is meaning** (wif / hex / dec / text / file / …). Do not merge them. `--encoding` remains **output** encoding only (`privkey`).
3. **Typed objects in the pipe always win.** `{` / `[` under `--in auto` is the object stream. `--from` applies to **bare lines**, not to an object’s `type`/`data`.
4. **`--from TYPE` is the override** where the guess is ambiguous (`privkey`, `pubkey`). Default is guess. Unknown `--from` is `invalid --from`. Cannot combine with generators like `--new` / `--sync`. `address` has no `--from` (`unknown option '--from'`).
5. **Silent SHA-256 is not universal.** Only `privkey` hashes leftover text without `--from`. `pubkey` and `address` require an explicit key. A WIF-shaped bad checksum is never hashed.
6. **`--out` / `--in` / `--network` / `--compressed` stay the global vocabulary.** Per-command flags (`--type`, `--match`, `--host`, `--sync`) stay per-command.

### Per-command

| Command | Kind | Stdin-only items? | `--from` values | Guess (bare line) | Silent SHA-256? |
|---|---|---|---|---|---|
| `privkey` | transformer / `--new` generator | yes (shipped) | `wif\|hex\|dec\|text\|file` | WIF → 64-hex → dec → text | yes (after guess) |
| `pubkey` | transformer | **yes** | `wif\|hex\|dec` (`hex` = 64-char priv or 66/130-char pub) | WIF → 64-hex priv → dec → 66/130 hex pub | **no**. Leftover text / `--from text\|file` are errors |
| `address` | transformer | **yes** | **none** (`--from` is unknown) | WIF → 66/130 hex pub | **no**. Unknown string errors. Typed `privkey` / `pubkey` objects still work (any encoding on the object). Bare 64-hex / decimal / leftover text are errors |
| `node` | parameterized one-shot | no | none | n/a | no. `--host HOST` required. Not a key pipe |
| `balance` query | transformer | **yes** | optional `address` | Base58Check / bech32 address only | **no** (unknown → error, not a hash). `--from-rpc` / `--from-chainstate` / `--path` are unknown |
| `balance --sync` | parameterized generator | no | none | n/a | no. `--host` stays a flag. `--sync` is RPC (create or catch up). Index is always `~/.btk/balance`. `--build` / `--update` are unknown |
| `config` | verb | n/a | none | n/a | no. `set`/`get`/`unset`/`dump` keep argv keys |

### Why pubkey does not silently hash

`pubkey` turns a key into a public key. It does not invent a secret from leftover text or a file. `printf test01 | btk pubkey` is `not a private or public key`. Hash a passphrase first with `btk privkey --from text`, then pipe the typed object.

### Why address has no `--from` and does not hash

Bare lines are only WIF or a 66/130-char hex pubkey — the two shapes do not overlap, so there is no override flag. `printf 1BgGZ9… | btk address` looking like “use this address” must not SHA-256 the string into a new key. Privkey’s job includes “string → secret”; address’s job is “key → script”. Hash a passphrase first with `btk privkey --from text`, then pipe the typed object.

### Why node/config keep non-pipe argv

`seed.bitcoin.sipa.be` and `rpc.host=127.0.0.1` are parameters, not a stream of objects. Continuity means “no naked *payloads*”, not “no words after the command.” `node` takes `--host` (D23). `config` keeps `set`/`get` argv keys.

### Naming clash

`--from` is stdin-item meaning only (`wif` / `hex` / …). Do not invent `--from rpc`. `--from-rpc` and `--from-chainstate` are unknown. `--sync` is RPC (create or catch up). `--build` and `--update` are unknown. `address` does not take `--from`.

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
- `source` is omitted unless `--source` is set. `--source` copies the input item (typed object as received, including a nested `source`, or a synthesized object for a bare string). `--match` and `--no-source` are unknown flags.
- `btk pubkey --help` is pinned in `test/cli/test_pubkey.py` (`PUBKEY_HELP`) and the command’s raw string.

## Phase 3 contract (as shipped)

This is what `btk address` actually does.

```text
btk address [--type p2pkh|p2wpkh|p2tr]...
            [--network mainnet|testnet]
            [--match REGEX] [--ignore-case]
            [--source] [--skip-incompatible]
```

- **Stdin only.** Positional keys are `provide input on stdin`.
- Accept typed `privkey` / `pubkey` objects (any encoding on the object) and bare WIF or 66/130-hex pub. No leftover-text or file hash. No `--from`.
- Bare-line guess: WIF → 66/130 hex pub. 64-hex, decimal, leftover text, and binary are `not a private or public key`. A WIF-shaped bad checksum is `invalid WIF checksum`. 64-hex is never an x-only key.
- `--from` is `unknown option '--from'`. `--count` is `unknown option '--count'`. `--stream` is accepted (already flushes).
- `--type` is repeatable; default one `p2wpkh`; emission order = flag order. Unknown style: `unknown address type`.
- Uncompressed + `p2wpkh`/`p2tr`: `uncompressed key cannot produce p2wpkh or p2tr`. P2PKH of an uncompressed key is allowed. `--skip-incompatible` drops those types for the current key and continues (empty stdout is still exit 0). Other errors stay fatal. `p2tr` is BIP-341 key-path, empty tree (`lift_x` + `TapTweak`); odd-Y and even-Y with the same X share an address.
- Network: WIF version → typed object `network` → `--network` → mainnet. Do not walk `source`. `--network` does not override a WIF version byte.
- `source` is included when `--match` is set, or when `--source` is set. Otherwise omitted. `--source` copies the input item (typed object as received, including a nested `source`). On a bare string it synthesizes a `privkey` (WIF) or `pubkey` (hex pub); `data` is the bare input. `--no-source` is an unknown flag.
- `--match` is POSIX ERE on `data`, compiled in the C locale (`REG_EXTENDED | REG_NOSUB`, plus `REG_ICASE` if `--ignore-case`). More than once: `cannot pass --match more than once`. Invalid pattern: `invalid match pattern`. All filtered → empty stdout, exit 0. Implies `source`.
- `btk address --help` is pinned in `test/cli/test_address.py` (`ADDRESS_HELP`) and the command’s raw string.

## Phase 4 contract (as shipped)

This is what `btk node` actually does.

```text
btk node --host HOST [--port 8333]
```

- Parameterized one-shot. Not a key pipe. `is_generator` is true so stdin is ignored.
- **`--host` is required.** Missing → `missing host`. A leftover positional is `provide input on stdin`.
- `--host` may include `:port` (one colon). Combined with `--port` → `port specified twice`. More than one colon → `invalid host`. Bad suffix → `invalid port`.
- IPv4 mainnet only (`getaddrinfo` `AF_INET`). Default port 8333. 15 s timeout on connect and read.
- Send `version` (protocol 70015, services 0, nonce 0, UA `/Bitcoin-Toolkit:4.0.0/`, height 0, relay 0, `addr_*` = IPv4-mapped `127.0.0.1:8333`). Read the peer’s `version`. Print the typed object. Close. Do **not** send `verack`.
- `--stream` → `node does not stream`. `--count` → `unknown option '--count'`. `--from` is unknown.
- `--out plain` prints `ip:port`. `--verbose` adds `raw` with `addr_recv`, `addr_trans`, `nonce` (decimal string; uint64 does not fit in a JSON number), `services_bits`.
- Service bits: named (`NODE_NETWORK`, `NODE_WITNESS`, …) plus unknown `BIT_<n>`, ascending bit order.
- Does not load `~/.btk/config.json`. `--network` is ignored.
- Offline: unit ser/de of the frozen 109-byte payload + a localhost mock peer in `test/cli/test_node.py`. Live handshake behind `BTK_RUN_NET=1` / `make test-net`.
- `btk node --help` is pinned in `test/cli/test_node.py` (`NODE_HELP`) and the command’s raw string.

## Phase 5 contract (as shipped)

This is what `btk balance` actually does.

```text
btk balance [--source] [--from address] [--skip-zero]
btk balance --sync [--host H] [--port P] [--rpc-auth USER:PASS]
```

- Query is a **transformer** on stdin. No positional addresses (`provide input on stdin`). `type=address` → `data`; `type=balance` → `address`; bare string → Base58Check / bech32 address. No leftover-text hash. Empty stdin → empty stdout, exit 0. Missing address → `sats: 0`. Missing database → `balance database not found (run btk balance --sync)`.
- `--source` copies the input item onto the output (typed `address` / `balance` as received, including a nested `source`; bare address → synthesized `{type, style, network, data}`). Without `--source` the field is omitted. `--source` + `--sync` is `cannot combine --sync and --source`. `--out plain` is still only `sats`.
- `--skip-zero` omits items whose `sats` is 0 (unknown / spent / never-seen). Empty stdout is still exit 0. Compatible with `--source` and `--out plain`. `--skip-zero` + `--sync` is `cannot combine --sync and --skip-zero`.
- Index is always `~/.btk/balance`. `--path` is unknown. No `balance.path` config key.
- `--sync` is RPC only. Missing/empty DB → walk `0 … tip`. Valid DB (`Mheight` + `Mtip`) → walk `Mheight+1 … tip` (already at tip → `complete`, exit 0). Non-empty junk → `balance database exists; rebuild with --sync --force`. Reorg → `reorg detected; rebuild with --sync --force`. `--force` only with `--sync`: wipe and walk from genesis. Wipe failure (DB locked) → `cannot remove balance database`.
- SIGINT / SIGTERM abort `--sync` within ~200 ms. Queued blocks are applied first. Stderr: `interrupted: height N`. Exit 1. The next `--sync` continues from `Mheight+1`.
- A missing prevout is skipped with no warning. We never store non-standard scripts, so those spends are expected. Do not spam `missing prevout` on incremental `--sync`.
- `--from address` is allowed on query. Other `--from` values are `invalid --from`. `--from` with `--sync` is `cannot combine --sync and --from`. `--source` with `--sync` is `cannot combine --sync and --source`. `--skip-zero` with `--sync` is `cannot combine --sync and --skip-zero`.
- `--build`, `--update`, `--from-rpc`, `--from-chainstate`, and `--chainstate` are unknown.
- `--host` / `--port` default to config `rpc.host` / `rpc.port` or `127.0.0.1` / `8332`. `--rpc-auth` is `user:pass` (Base64 at request time). No cookie file.
- Progress on stderr (`syncing:` / `complete:`). Query is read-only.
- `btk balance --help` is pinned in `test/cli/test_balance.py` (`BALANCE_HELP`) and the command’s raw string.
- Offline: unit CompactSize / A.10 txid / a tiny DB writer. CLI uses a localhost mock JSON-RPC server. SIGINT abort is tested against a hanging listener. No live bitcoind in `make test`.

## Phase 6 contract (as shipped)

This is what `btk config` actually does.

```text
btk config set <key>=<value>
btk config unset <key>
btk config get <key>
btk config dump
```

- Verb, not a pipe. `is_generator` is true so stdin is ignored. Verbs and keys stay on argv.
- Keys: `rpc.host`, `rpc.port`, `rpc.auth`. Anything else on `set`/`get`/`unset` is `unknown config key 'foo'`. No `balance.path`.
- Path: `--config`, else `$BTK_CONFIG`, else `~/.btk/config.json`. Nested JSON. Types: host/auth string, port JSON number 1–65535. Unknown on-disk fields are ignored (and preserved on rewrite).
- Create the file (mode `0600`) and parents (`0700`) **only on `set`**. Missing file: `dump` → `{"type":"config"}` exit 0; `get`/`unset` → `no such key` exit 1. None of those mkdir.
- `get` of a present key emits a one-key `config` object. `--out plain` is the raw stored value (`8332` for the port). `rpc.auth` is always eight asterisks. Missing key in an existing file: `no such key`.
- `dump` emits the typed `config` object (dotted keys). `--out plain` is `key=value` lines in order `rpc.host`, `rpc.port`, `rpc.auth`. Auth redacted; omitted when unset. No `--show-secrets`.
- `set` / `unset` print nothing. `set` without `=` is `expected key=value`. Bad port is `invalid rpc.port`. Extra argv is `unexpected argument`. No verb is `expected set, get, unset, or dump`. Unknown verb is `unknown config verb '…'`.
- `--stream` → `config does not stream`. `--count` → `unknown option '--count'`. `--from` is unknown.
- Load config **only** for `config` and `balance`. Phases 1–4 must not open the file. No man page.
- `btk config --help` is pinned in `test/cli/test_config.py` (`CONFIG_HELP`) and the command’s raw string.

## How to continue

The 4.0.0 command set is complete (`privkey`, `pubkey`, `address`, `node`, `balance`, `config`). There is no `help` or `version` command. Use `--help` / `--version`.

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
make test-net      # live P2P; requires BTK_RUN_NET=1 / network
make clean && make test   # after header/layout changes
```
