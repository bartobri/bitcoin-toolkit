# Agent notes — Bitcoin Toolkit 4.0.0

This file is the source of truth for **development**. [README.md](README.md)
is the source of truth for **new users**. Read this first when changing the
program. Do not restore Bitcoin Toolkit 3.1.2 sources, tests, man pages,
cJSON, or QR; tag `legacy/3.1.2` is history only.

There is no `help` or `version` command. Use `--help` / `--version`. Version
is `4.0.0` (`src/version.hpp`).

## Keep README.md and AGENTS.md current

**Every commit that would make either file stale must update it in the same
change, even when the user does not ask.** Do not wait for a docs pass.

Update **README.md** when the user-facing story changes: what the program is,
how it works, commands, examples, build/install, warnings, or license.

Update **AGENTS.md** when the development contract changes: CLI behavior,
input/output, algorithms, layout, tests, dependencies, conventions, asserted
error strings, help text, or golden vectors.

README stays high-level and interesting. This file stays complete enough that
an agent can implement or review a change without inventing a contract. If
they would disagree after the commit, the commit is not done.

## Product

`btk` is a Git-style Unix CLI: one short-lived process, one command per
invocation, composition via pipes. Default wire format is typed NDJSON.

| Command | Kind | Job |
|---|---|---|
| `privkey` | transformer / `--new` generator | Create or convert private keys |
| `pubkey` | transformer | Derive or recompress public keys |
| `address` | transformer | Derive P2PKH / P2WPKH / BIP-341 P2TR |
| `node` | parameterized one-shot | IPv4 mainnet `version` handshake |
| `balance` query | transformer | Read `~/.btk/balance` |
| `balance --sync` | parameterized generator | Fill or catch up that index from Core RPC |
| `config` | verb | `set` / `get` / `unset` / `dump` RPC defaults |

Out of scope (do not add unless the user asks): QR, HD / BIP-32 / BIP-39,
PSBT, message sign, tx build, P2SH/P2WSH generation, IPv6 P2P, signet,
regtest, Windows, CMake, OpenSSL, a `help`/`version` command, cookie-file
RPC auth, `balance.path`, cloning 3.1.2 flags.

## Hard rules

1. Item payloads are never positionals. Leftover argv is `provide input on stdin`. Flag arguments (`--count 5`, `--host x`) stay.
2. `--in` is framing (`auto` / `ndjson` / `json` / `plain`). `--from` is meaning (`wif` / `hex` / …). `--encoding` is output only (`privkey`).
3. Typed JSON objects in the pipe always win. `--from` applies to **bare lines**, not to an object’s `type`/`data`.
4. Silent SHA-256 is **privkey only**. `pubkey` / `address` / `balance` require an explicit key or address. A WIF-shaped bad checksum is never hashed.
5. `--from` is stdin-item meaning only. Do not invent `--from rpc`. `--from-rpc`, `--from-chainstate`, `--build`, `--update`, and `--path` are unknown. `address` does not take `--from`.
6. Load `~/.btk/config.json` **only** for `config` and `balance`. `privkey` / `pubkey` / `address` / `node` must not open `~/.btk`.
7. Errors: `BtkError(command, message)` → `btk <command>: <message>` on stderr, exit 1. Never put WIF, hex keys, passphrases, or `rpc.auth` in messages.
8. Network is per object, never process-global. Do not walk `source` for network. `--network` does not override a WIF version byte. `node` ignores `--network`.
9. Help text is pinned **byte-for-byte** in `test/cli/test_<cmd>.py` and the command’s raw string. Edit both together. Man pages wrap the same ideas (no `config` man page; help never execs `man`).
10. `make test` is the gate (unit + offline CLI). No network. Live P2P is `make test-net` / `BTK_RUN_NET=1`.

## Layout

```
src/main.cpp
src/version.hpp
src/cli/          dispatcher, options, io, output
src/cmd/          command.hpp, privkey, pubkey, address, node, balance, config
src/core/         hash, hex, base58, bech32, json_io, secp, random, privkey, pubkey, address, network.hpp
src/chain/        compactsize, transaction, script, balance_db, indexer
src/net/          p2p, jsonrpc
src/util/         error, config (load + save), interrupt
man/btk-privkey.1 man/btk-pubkey.1 man/btk-address.1 man/btk-node.1 man/btk-balance.1
test/unit/        hash, hex, base58, privkey, pubkey, bech32, address, p2p, compactsize, tx, balance_db
test/cli/         test_scaffold.py, test_privkey.py, test_pubkey.py, test_address.py,
                  test_node.py, test_balance.py, test_config.py
test/runner.py    discovers test/cli/test_*.py
third_party/picojson/
```

Register new commands in `register_builtin_commands()` (`src/cli/dispatcher.cpp`).
Add the `.cpp` to `SRC` in the Makefile. Unit tests are extra `bin/test_*` targets.

**Makefile has no header deps.** Changing a widely included header (especially
`Options` layout) without touching every `.cpp` can produce silent ABI/stack
bugs. `make clean && make test` after those edits.

Commands depend on libraries; libraries do not know about CLI flags.
`src/cli/io.cpp` owns stdin framing and binary-prefix detection. Commands must
not assume positionals.

## Build and test

C++17, GNU Makefile, no Boost, no CMake. Unix only.

- Required: `libsecp256k1` (`-lsecp256k1`). Makefile probe fails with the
  apt/dnf/brew line.
- Optional: LevelDB. Missing → `-DBTK_NO_LEVELDB` and `btk balance` prints
  `this build was compiled without LevelDB (install libleveldb-dev and rebuild)`.
- Hashes: SHA-256 and RIPEMD-160 in `src/core/hash.cpp`. No OpenSSL.
- JSON: vendored picojson pin `111c9be5188f7350c2eac9ddaedd8cca3d7bf394`
  (see `third_party/README.md`).
- No GMP. 256-bit decimal is a 32-byte ×10/÷10 loop in `src/core/privkey.cpp`.
- CSPRNG: `getentropy` else `/dev/urandom`. Short read is `could not read CSPRNG`.
- `prefix ?= /usr/local`. `make install` installs `bin/btk` and `man/btk*.1`.

```sh
make
make test          # unit + offline CLI
make test-unit
make test-cli
make test-net      # live P2P; requires BTK_RUN_NET=1 / network
make clean && make test   # after header/layout changes
```

Tests compare parsed JSON objects, not string tables (picojson key order is
not a contract). CLI tests spawn `bin/btk`. Unit tests are C++ with
`test/unit/check.hpp` (no gtest). Default tests are offline; `test_node.py`
uses a localhost mock peer; `test_balance.py` uses a localhost mock JSON-RPC
server. SIGINT abort is tested against a hanging listener. No live bitcoind
in `make test`.

## Dispatcher

`main` maps exceptions to exit 1. `dispatch`:

- No argv command → overview help on **stderr**, exit 1.
- `btk --help` → overview on stdout, exit 0.
- `btk --version` → typed `version` object (`version`, `secp256k1`, `leveldb`).
  `--out plain` prints `4.0.0`.
- Unknown command: `btk: unknown command 'foo'` plus `See 'btk --help'…`.
- Global flags may appear before or after the command (`btk --config PATH privkey --new`), except `--help`/`--version` which also work with no command.
- Do not create `~/.btk` on unknown command or failed option parse.
- After a successful parse, load config **only** for `config` and `balance`. Missing file → compiled defaults (no mkdir). Invalid JSON / wrong type for a known key → `invalid config file`. Unknown on-disk fields are ignored (and preserved on rewrite).
- Config path: `--config` else `$BTK_CONFIG` else `$HOME/.btk/config.json`. Relative paths are cwd. Unset `HOME` when needed: `HOME is not set`.
- Generators (`is_generator`): `--new` repeats `--count` times (default 1); `--stream` without `--count` is infinite until SIGINT; `--stream --count N` is finite N. Other generators (`node`, `config`, `balance --sync`, `privkey --from file`) run once.
- Transformers: one input item → zero or more output objects, written before the next item is read. Empty stdin → empty stdout, exit 0.
- SIGINT / SIGTERM set `stop_requested()` (`src/util/interrupt.cpp`).

`Command` interface is in `src/cmd/command.hpp`. `Options` is a flat struct
in `src/cli/options.hpp` (global + per-command flags). Unknown flags are
errors. `getopt_long` is fine; `--` stops option parsing. `POSIXLY_CORRECT`
stops at the first non-option (warn in help).

### Command kinds

| Kind | Argv | Input stream? |
|---|---|---|
| **Transformer** | flags only | yes — items on stdin |
| **Generator** | flags (`--new`, `--sync`, …) | no (or raw stdin if `--from file`) |
| **Parameterized one-shot** | flags for host/port | no |
| **Verb** | subcommand + key names | no |

Apply `--from` only where the input is a stream of keys **and** the bare-line
guess is ambiguous. Do not force it onto hosts, paths, verbs, or `address`
(WIF vs hex pub is unambiguous). Continuity means “no naked *payloads*”, not
“no words after the command.”

### Flush

1. Transformers are always incremental.
2. `--out ndjson` (default) and `--out plain`: `fflush` after every object / line.
3. `--out json`: one pretty JSON value (object if 1, array if N) at the end, **unless** `--stream`, in which case behave as ndjson.
4. TTY does not change the contract. No hidden pretty-print when isatty.

A producer that writes one NDJSON object and sleeps must cause a consumer
reading an object stream (`--in auto` seeing `{`, or `--in ndjson`) to emit
before the producer exits. A pretty JSON **array** may be parsed as one
picojson value; it is not required to yield elements before the array closes.

### Stdin (`--in auto`, pipe, not a TTY)

I/O peeks 8KiB:

1. **Binary** (NUL, C0 controls other than tab/LF/CR, or invalid UTF-8) → one raw item. `privkey` SHA-256s it (same as `--from file`). Other transformers do not.
2. Else `{` → incremental object stream; `[` → one JSON array; else **plain lines**.
3. A TTY stays line-oriented.

`--from text|wif|hex|dec` with `--in auto` is coerced to `--in plain`.
`--from file` does not read the object stream.

`--in json`: one complete picojson value (object or array). Concatenated
`{}{}` is not required; use `ndjson` / `auto`.

## Typed pipe format

Every default-mode record is one JSON **object** with `type`:
`privkey` | `pubkey` | `address` | `node` | `balance` | `version` | `config`.

Unknown `type` on stdin: error, except where a command lists accepted types.
Extra fields are ignored on input and must not be invented on output.

JSON integers (`sats`, `port`, `height`, `protocol`, `timestamp`, `rpc.port`)
are written via `set_uint64` as JSON numbers (picojson `double`). Values we
emit fit in 2^53. `node` `nonce` is a **decimal string** (uint64 does not fit
in a JSON number). Decimal privkey `data` is a JSON **string**, never a JSON
number.

`--out plain` primary:

| Command | Primary |
|---|---|
| `privkey` | `data` |
| `pubkey` | `data` |
| `address` | `data` |
| `balance` | `sats` as decimal digits |
| `--version` | `version` |
| `node` | `ip:port` |
| `config dump` | `key=value` lines |
| `config get` | the raw value (redacted if `rpc.auth`) |

### `privkey`

```json
{"type":"privkey","encoding":"wif","network":"mainnet","compressed":true,"data":"KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn"}
```

`encoding`: `wif` (default on output), `hex`, `dec`. Hex `data` is 64 lowercase
chars, no trailing `01`/`00` — compression is the boolean. Decimal `data` is a
digit string with no leading zeros.

`source` (only with `--source`) is **origin**, not a copy of a prior item:

| How | `source` |
|---|---|
| `--new` | `{"from":"new"}` |
| Bare line / `--from wif\|hex\|dec\|text` | `{"from":"<kind>","data":"<the line>"}` |
| `--from file` or binary stdin | `{"from":"file"}` — no path, no bytes |
| Typed `privkey` object | copied as received (including a nested `source`) |

### `pubkey`

```json
{"type":"pubkey","encoding":"hex","network":"mainnet","compressed":true,"data":"0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798"}
```

`encoding` is always `hex`. `data` is 66-char (`02`/`03`) or 130-char (`04`)
lowercase hex. Optional `source` is the **input item** when `--source` is set.

### `address`

```json
{"type":"address","style":"p2wpkh","network":"mainnet","data":"bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4"}
```

`style`: `p2pkh` | `p2wpkh` | `p2tr` on generate; `p2sh` | `p2wsh` appear on
**balance** output only (indexed, not generated). Optional `source` is the
input item when `--source` or `--match` is set.

### `node`

```json
{"type":"node","host":"seed.bitcoin.sipa.be","ip":"1.2.3.4","port":8333,"protocol":70016,"user_agent":"/Satoshi:24.0.1/","height":786299,"services":["NODE_NETWORK","NODE_WITNESS"],"relay":true,"timestamp":1682030946}
```

### `balance`

```json
{"type":"balance","address":"1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa","sats":0}
```

Unknown address → `sats: 0`, not an error. Optional `source` is the input item
when `--source` is set.

Query input: `type=address` → `data`; `type=balance` → `address`; other typed
object → `expected an address`; bare string → parse as a Bitcoin address.

### `version`

```json
{"type":"version","version":"4.0.0","secp256k1":true,"leveldb":true}
```

`leveldb` is whether this binary was linked with LevelDB.

### `config`

```json
{"type":"config","rpc.host":"127.0.0.1","rpc.port":8332,"rpc.auth":"********"}
```

`rpc.auth` is **always** eight asterisks when present; omitted when unset.

### Provenance

`--source` is opt-in. Without it the field is omitted. `--match` implies
`--source` on `address`. `--no-source` is unknown.

On `pubkey` / `address` / `balance`, `--source` copies the input item (typed
object as received, including a nested `source`). A bare string is synthesized:
WIF → `privkey`; 66/130 hex pub → `pubkey`; address → `{type, style, network, data}`.
Synthetic `data` is the bare input; do not re-encode.

Each stage that opts in extends the chain; a stage that does not drops it.
Network is still not walked from `source`. Vanity that needs the spendable key
is `privkey | address --match` (not via `pubkey`).

## Shared `--from` / guess

| Command | `--from` values | Bare-line guess | Silent SHA-256? |
|---|---|---|---|
| `privkey` | `wif\|hex\|dec\|text\|file` | WIF → 64-hex → dec → text | **yes** (after guess) |
| `pubkey` | `wif\|hex\|dec` (`hex` = 64-char priv or 66/130-char pub) | WIF → 64-hex priv → dec → 66/130 hex pub | **no** |
| `address` | **none** (`unknown option '--from'`) | WIF → 66/130 hex pub | **no**. Bare 64-hex / decimal / leftover text error |
| `balance` query | optional `address` | Base58Check / bech32 address | **no** |
| `node` / `config` / `balance --sync` | none | n/a | no |

Unknown `--from` is `invalid --from`. Cannot combine with `--new` / `--sync`.
64-digit all-numeric is hex, not decimal. A 66- or 130-digit all-numeric
string on `pubkey` is decimal (guessed before hex pub).

`--from` cannot combine with generators like `--new` / `--sync`.

## Errors

| Code | When |
|---|---|
| 0 | Success, including “`--match` filtered everything” or `--skip-zero` emptied stdout |
| 1 | Any error |
| 130 | SIGINT on a `--stream` generator is optional; tests must not require 130 |

Prefix is `btk <command>:`. RAII owners (`SecpContext`, sockets, LevelDB,
files) run destructors on exception unwind.

Global options: `-h/--help`, `-V/--version`, `--config PATH`, `-n/--network`,
`-o/--out`, `--in`, `-s/--stream`, `-c/--count`, `--from`. Per-command flags
stay per-command (`--type`, `--match`, `--host`, `--sync`, `--encoding`, …).

## `btk privkey`

```text
btk privkey --new [--count N] [--stream]
            [--encoding wif|hex|dec] [--network mainnet|testnet]
            [--compressed | --uncompressed] [--source]
btk privkey [--encoding wif|hex|dec] [--network mainnet|testnet]
            [--compressed | --uncompressed]
            [--from wif|hex|dec|text|file] [--source]
```

- Stdin only. `--new` and `--from` cannot be combined (`cannot combine --new and --from`).
- `--count` / `--stream` require `--new` (`--count requires --new`, `--stream requires --new`).
- `--count` default 1; must be `≥ 1` (`invalid --count`).
- Both `--compressed` and `--uncompressed` → two objects (compressed first). Default compressed.
- Secrets checked with `secp256k1_ec_seckey_verify`. `0` / `≥ n` → `private key out of range` (not a hash).
- WIF-shaped bad checksum → `invalid WIF checksum` (not a hash).
- `--from wif|hex|dec`: parse each line as that type; error if it does not fit (`invalid hex private key`, `invalid decimal private key`, …).
- `--from text`: SHA-256 each line (how to force `"1"` to be a passphrase).
- `--from file`: SHA-256 entire stdin as raw bytes, one key (generator).
- Decimal emit has no leading zeros (`1` not `01`). Parse strips leading zeros (`001` is secret 1). Force decimal on a 64-digit string with a typed object `"encoding":"dec"`.
- `--network` re-encodes WIF to that network. Hex/dec items pick up `--network`, default mainnet.
- `--from text` / `--from file` / leftover-text / binary hash is treated as compressed mainnet unless flags say otherwise.
- WIF payload: `version || 32-byte scalar || [0x01 if compressed]`. Version mainnet `0x80`, testnet `0xEF`. Checksum: first 4 of HASH256(payload). Do not pattern-match the first Base58 character as the decoder.
- Hex output lowercase; hex input case-insensitive.
- `--no-source` is unknown.

Help is pinned in `test/cli/test_privkey.py` (`PRIVKEY_HELP`).

## `btk pubkey`

```text
btk pubkey [--compressed | --uncompressed]
           [--from wif|hex|dec] [--source]
```

- Stdin only. Accept typed `privkey` / `pubkey` and bare WIF / 64-hex priv / decimal / 66/130-hex pub.
- No leftover-text or file hash. No `--from text` / `--from file`. Fail: `not a private or public key`.
- From a private key: `secp256k1_ec_pubkey_create` + serialize. From a pubkey: parse + recompress.
- Default compression follows the input; flags override; both flags → two objects.
- `network`: typed input → WIF version → `--network` → mainnet.
- `--match` is unknown. `--no-source` is unknown.

Why it does not silently hash: `pubkey` turns a key into a public key. It does
not invent a secret. `printf test01 | btk pubkey` is `not a private or public key`.
Hash first with `btk privkey --from text`.

Help is pinned in `test/cli/test_pubkey.py` (`PUBKEY_HELP`).

## `btk address`

```text
btk address [--type p2pkh|p2wpkh|p2tr]...
            [--network mainnet|testnet]
            [--match REGEX] [--ignore-case]
            [--source] [--skip-incompatible]
```

- Stdin only. Accept typed `privkey` / `pubkey` (any encoding on the object) and bare WIF or 66/130-hex pub.
- `--from` is `unknown option '--from'`. `--count` is unknown. `--stream` is accepted (already flushes).
- Bare 64-hex, decimal, leftover text, and binary are `not a private or public key`. 64-hex is never an x-only key. Negative: G’s x-only hex `79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798` must **not** produce G’s P2TR. Feed a secret as a typed `privkey`; feed an x-only key as a `pubkey` with 66-char `02`/`03` `data`.
- `--type` is repeatable; default one `p2wpkh`; emission order = flag order. Unknown style: `unknown address type`.
- Uncompressed + `p2wpkh`/`p2tr`: `uncompressed key cannot produce p2wpkh or p2tr`. P2PKH of an uncompressed key is allowed. `--skip-incompatible` drops those types for the current key and continues (empty stdout is still exit 0). Other errors stay fatal.
- No `--bech32m` flag. `p2tr` is BIP-341 key-path, empty tree.
- Network: WIF version → typed object `network` → `--network` → mainnet. Do not walk `source`.
- `--match` is POSIX ERE on `data`, C locale (`REG_EXTENDED | REG_NOSUB`, plus `REG_ICASE` if `--ignore-case`). `setlocale`/`uselocale` so `LC_COLLATE` cannot change `^1bri`. More than once: `cannot pass --match more than once`. Invalid: `invalid match pattern`. All filtered → empty stdout, exit 0. Implies `source`.

Why no `--from` and no hash: the two bare shapes do not overlap. `printf 1BgGZ9… | btk address` must not SHA-256 an address-shaped string into a new key.

Help is pinned in `test/cli/test_address.py` (`ADDRESS_HELP`).

### Scripts

| `--type` | Construction |
|---|---|
| `p2pkh` | Base58Check(`ver \|\| HASH160(serialized pubkey)`). `ver` = `0x00` main / `0x6F` test |
| `p2wpkh` | Bech32 (BIP-173), witness v0, 20-byte HASH160(**compressed** pubkey). HRP `bc` / `tb` |
| `p2tr` | Bech32m (BIP-350), witness v1, 32-byte **tweaked** x-only output key. HRP `bc` / `tb` |

A `p2tr` address is 62 characters (`bc1p` + 58). It is **not** a 42-character HASH160 bech32m string.

### BIP-341 `p2tr` (empty script tree)

1. Require a compressed (or compressible) pubkey. `X` = 32-byte x-coordinate. Same `X` → same address (`lift_x` uses even Y, BIP-340). Odd-Y (`03`) and even-Y (`02`) with the same X share an address.
2. Internal key `P = lift_x(X)`.
3. `t = int(tagged_hash("TapTweak", X))` where `tagged_hash(tag, msg) = SHA256(SHA256(tag) || SHA256(tag) || msg)`. Empty Merkle root: do **not** append a script-root. If `t ≥ n`, `taproot tweak out of range`.
4. `Q = P + t·G` via `secp256k1_xonly_pubkey_parse` + `secp256k1_xonly_pubkey_tweak_add` (or `secp256k1_keypair_xonly_tweak_add` from a secret). Do not `ec_pubkey_tweak_add` the odd point.
5. Program = `x(Q)` (32 bytes). Encode witness version 1 with **bech32m** (checksum xor `0x2bc830a3`).

BIP-350’s `bc1p0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7vqzk5jj0` is the **untweaked** `x(G)` encoded as witness v1 — a bech32m self-test, **not** a BIP-341 spendable P2TR for secret `1`. `btk address --type p2tr` must emit the tweaked address in the goldens below.

## `btk node`

```text
btk node --host HOST [--port 8333]
```

- Parameterized one-shot. `is_generator` is true so stdin is ignored.
- `--host` is required (`missing host`). Leftover positional is `provide input on stdin`.
- `--host` may include `:port` (one colon). Combined with `--port` → `port specified twice`. More than one colon → `invalid host`. Bad suffix → `invalid port`.
- IPv4 mainnet only (`getaddrinfo` `AF_INET`). Default port 8333. 15 s timeout on connect and read.
- Send `version` (protocol 70015, services 0, nonce 0, UA `/Bitcoin-Toolkit:4.0.0/`, height 0, relay 0, `addr_*` = IPv4-mapped `127.0.0.1:8333`). Read the peer’s `version`. Print the typed object. Close. Do **not** send `verack`.
- `--stream` → `node does not stream`. `--count` → `unknown option '--count'`. `--from` is unknown.
- `--out plain` prints `ip:port`. `--verbose` adds `raw` with `addr_recv`, `addr_trans`, `nonce` (decimal string), `services_bits`.
- Service bits, ascending: 0 `NODE_NETWORK`, 1 `NODE_GETUTXO`, 2 `NODE_BLOOM`, 3 `NODE_WITNESS`, 4 `NODE_XTHIN`, 6 `NODE_COMPACT_FILTERS`, 10 `NODE_NETWORK_LIMITED`. Unknown set bits as `BIT_<n>`. Do not omit unknown bits.
- Does not load config. `--network` is ignored.

Help is pinned in `test/cli/test_node.py` (`NODE_HELP`).

### `version` payload (109 bytes)

Multi-byte integers little-endian **except** `addr_*` ports (network byte order).

| Offset | Size | Field | We send |
|---|---|---|---|
| 0 | 4 | version int32 LE | `70015` (`7f 11 01 00`) |
| 4 | 8 | services uint64 LE | `0` |
| 12 | 8 | timestamp int64 LE | `time(NULL)` at runtime; tests use frozen hex |
| 20 | 8 | addr_recv.services | `0` |
| 28 | 16 | addr_recv.ip | IPv4-mapped `127.0.0.1` |
| 44 | 2 | addr_recv.port | BE `8333` = `20 8d` |
| 46 | 8 | addr_trans.services | `0` |
| 54 | 16 | addr_trans.ip | same as recv |
| 70 | 2 | addr_trans.port | BE `20 8d` |
| 72 | 8 | nonce uint64 LE | `0` |
| 80 | 1+ | user_agent | CompactSize `17` + `/Bitcoin-Toolkit:4.0.0/` |
| 104 | 4 | start_height int32 LE | `0` |
| 108 | 1 | relay | `0` |

P2P framing: magic `f9 be b4 d9`; 12-byte command; uint32 LE length; checksum = first 4 of HASH256(payload). UA is Bitcoin CompactSize, not Core VARINT.

Frozen unit-test vector (timestamp `1700000000`, nonce `0` — tests **must not** call `time(NULL)`). Full message including 24-byte header (checksum `d2a9d2ea`):

```
f9beb4d976657273696f6e00000000006d000000d2a9d2ea7f110100000000000000000000f1536500000000000000000000000000000000000000000000ffff7f000001208d000000000000000000000000000000000000ffff7f000001208d0000000000000000172f426974636f696e2d546f6f6c6b69743a342e302e302f0000000000
```

Payload only:

```
7f110100000000000000000000f1536500000000000000000000000000000000000000000000ffff7f000001208d000000000000000000000000000000000000ffff7f000001208d0000000000000000172f426974636f696e2d546f6f6c6b69743a342e302e302f0000000000
```

## `btk balance`

```text
btk balance [--source] [--from address] [--skip-zero]
btk balance --sync [--host H] [--port P] [--rpc-auth USER:PASS]
```

- Query is a transformer on stdin. Empty stdin → empty stdout, exit 0. Missing address → `sats: 0`. Missing database → `balance database not found (run btk balance --sync)`.
- `--source` copies the input item. `--source` + `--sync` is `cannot combine --sync and --source`. `--out plain` is still only `sats`.
- `--skip-zero` omits `sats == 0` (unknown / spent / never-seen). Empty stdout is still exit 0. `--skip-zero` + `--sync` is `cannot combine --sync and --skip-zero`.
- Index is always `~/.btk/balance`. `--path` is unknown. No `balance.path` config key.
- `--from address` is allowed on query. Other `--from` values are `invalid --from`. `--from` + `--sync` is `cannot combine --sync and --from`.
- `--build`, `--update`, `--from-rpc`, `--from-chainstate`, `--chainstate` are unknown.
- `--host` / `--port` default to config `rpc.host` / `rpc.port` or `127.0.0.1` / `8332`. `--rpc-auth` is `user:pass` (split on the **first** colon; Base64 at request time). No cookie file.
- Progress on stderr (`syncing:` / `complete:`). Query is read-only (no write handle).
- LevelDB missing at compile time: `this build was compiled without LevelDB (install libleveldb-dev and rebuild)`.

Help is pinned in `test/cli/test_balance.py` (`BALANCE_HELP`).

### `--sync`

| State of `~/.btk/balance` | Action |
|---|---|
| Missing or empty directory | Create the DB and walk `0 … tip` |
| Valid index (`Mheight` + `Mtip`) | Walk `Mheight+1 … tip`. Already at tip → `complete`, exit 0 |
| Non-empty junk | `balance database exists; rebuild with --sync --force` |
| Reorg (stored hash at `Mheight` ≠ RPC hash) | `reorg detected; rebuild with --sync --force` |
| `--force` | Wipe and walk `0 … tip`. `--force` only with `--sync` (`--force requires --sync`). Wipe failure (DB locked) → `cannot remove balance database` |

SIGINT / SIGTERM abort within ~200 ms. Queued blocks are applied first. Stderr:
`interrupted: height N`. Exit 1. The next `--sync` continues from `Mheight+1`.

A missing prevout is skipped with no warning. We never store non-standard
scripts, so those spends are expected. Do not spam `missing prevout`.

RPC: HTTP, `Content-Type: application/json`, `Authorization: Basic …`.
Methods: `getblockcount`, `getblockhash height`, `getblock hash 0` (raw hex).
One fetch thread, one parse thread, one DB-writer thread; mutex + condvar;
max queue depth 100. A pruned node cannot complete a genesis walk. A mainnet
first sync takes days; later runs are incremental.

Progress:

```text
syncing: height 800000/850000 (94.1%)
complete: height 850000
```

`\r` on the progress line is fine. Final `complete` ends with `\n`.

### Storage

LevelDB, not compatible with 3.1.2. The writer library may take a directory
for tests; the CLI never does.

| Key | Value |
|---|---|
| `A` \|\| UTF-8 address | `uint64` satoshis, little-endian |
| `O` \|\| 32-byte txid (internal/LE) \|\| `uint32` vout LE | CompactSize(addr len) \|\| addr \|\| `uint64` amount LE |
| `Mheight` | `uint32` LE last consumed height |
| `Mtip` | 32-byte tip hash (internal) |

**txid is BIP-141:** HASH256 of the **non-witness** serialization
(`nVersion || vin || vout || nLockTime`). Never hash marker/flag/witness.
Display-hex txids in logs are byte-reversed; keys store internal order.

Who gets an `A` row (mainnet encoding in 4.0.0):

| Script | Address |
|---|---|
| `OP_DUP OP_HASH160 20 <h> OP_EQUALVERIFY OP_CHECKSIG` | P2PKH |
| `OP_HASH160 20 <h> OP_EQUAL` | P2SH (index only) |
| `OP_0 20 <h>` | P2WPKH |
| `OP_0 32 <h>` | P2WSH (index only) |
| `OP_1 32 <x>` | P2TR: encode the 32-byte `x` **as-is** (bech32m v1). Do **not** apply a BIP-341 tweak — the program on chain is already the output key |
| `<33/65-byte pubkey> OP_CHECKSIG` | P2PKH of that pubkey (historical P2PK) |

Anything else is skipped (not an error).

For each block, for each tx, for each input (skip coinbase): look up
`O[prevout]`; if found, debit that address (floor at 0) and delete the
outpoint. For each recognized output: credit `A[addr]`, write `O[this_outpoint]`.

## `btk config`

```text
btk config set <key>=<value>
btk config unset <key>
btk config get <key>
btk config dump
```

- Verb, not a pipe. `is_generator` is true. Verbs and keys stay on argv.
- Keys: `rpc.host`, `rpc.port`, `rpc.auth`. Anything else on `set`/`get`/`unset` is `unknown config key 'foo'`. No `balance.path`.
- Types: host/auth string, port JSON number 1–65535 (`invalid rpc.port`).
- Create the file (mode `0600`) and parents (`0700`) **only on `set`**. Missing file: `dump` → `{"type":"config"}` exit 0; `get`/`unset` → `no such key` exit 1. None of those mkdir.
- `get` of a present key emits a one-key `config` object. `--out plain` is the raw stored value (`8332` for the port). `rpc.auth` is always eight asterisks. Missing key in an existing file: `no such key`.
- `dump` emits the typed `config` object (dotted keys). `--out plain` is `key=value` lines in order `rpc.host`, `rpc.port`, `rpc.auth`. Auth redacted; omitted when unset. No `--show-secrets`.
- `set` / `unset` print nothing. `set` without `=` is `expected key=value`. Extra argv is `unexpected argument`. No verb is `expected set, get, unset, or dump`. Unknown verb is `unknown config verb '…'`.
- `--stream` → `config does not stream`. `--count` → `unknown option '--count'`. `--from` is unknown.
- No man page.

On-disk (`~/.btk/config.json`):

```json
{"rpc":{"host":"127.0.0.1","port":8332,"auth":"alice:s3cret"}}
```

Help is pinned in `test/cli/test_config.py` (`CONFIG_HELP`).

## Crypto

- `secp256k1_context_create(SECP256K1_CONTEXT_NONE)` (or `SIGN|VERIFY` on older headers).
- `secp256k1_ec_seckey_verify`, `secp256k1_ec_pubkey_create`, `secp256k1_ec_pubkey_parse` / `serialize`.
- Taproot: `secp256k1_xonly_pubkey_parse`, `secp256k1_xonly_pubkey_tweak_add`. Distro packages enable the extrakeys module; a package built without it will fail to link `btk address --type p2tr`.
- Curve order `n = FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141`.
- `SHA256`, `RIPEMD160`, `HASH256 = SHA256∘SHA256`, `HASH160 = RIPEMD160∘SHA256`, `tagged_hash` as BIP-340. Test against the goldens below.
- 32-byte scalars are `std::array<uint8_t,32>`. Range checks go through `secp256k1_ec_seckey_verify`.

### CompactSize (Bitcoin P2P / txs — not Core chainstate VARINT)

| n | Encoding |
|---|---|
| `< 253` | 1 byte `n` |
| `≤ 0xffff` | `0xfd` + uint16 LE |
| `≤ 0xffffffff` | `0xfe` + uint32 LE |
| else | `0xff` + uint64 LE |

Goldens: `0 → 00`, `23 → 17`, `252 → fc`, `253 → fd fd 00`.

### Block / tx layout (RPC `--sync` and BIP-141 txid)

Block: `int32 version | 32-byte prev | 32-byte merkle | uint32 time | uint32 bits | uint32 nonce | CompactSize tx_count | tx…`.

Transaction:

```
int32 nVersion
[if dummy vin CompactSize == 0 and flags != 0: marker 0x00, flag, then real vin]
CompactSize vin_count
repeat vin: 32-byte prev txid (internal) | uint32 vout | CompactSize scriptsig | uint32 sequence
CompactSize vout_count
repeat vout: int64 value | CompactSize spk | spk
[if flag & 1: per-input witness = CompactSize item_count, each item CompactSize+bytes]
uint32 nLockTime
```

**txid** = HASH256(`nVersion || vin || vout || nLockTime`). **wtxid** = HASH256(full serialization). Indexer keys `O` with the **internal** txid.

## Golden vectors

Hex is lowercase on emit; input accepts both. These are the acceptance oracles
(also in `test/unit/` and CLI tests). A commonly pasted testnet uncompressed
WIF `91avARGdfge8E4tZfYLoxeJ5sGBdNJQH4kvjJoQFacbgx3cTMqe` decodes to secret
**3**, not 1 — do not use it.

### Hashes

| Input | Algorithm | Digest |
|---|---|---|
| `abc` | SHA-256 | `ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad` |
| `abc` | RIPEMD-160 | `8eb208f7e05d987a9b044a8e98c6b087f15a0bfc` |
| `abc` | HASH256 | `4f8b42c22dd3729b519ba6f68d2da7cc5b2d606d05daed5ad5128cc03e6c6358` |
| `abc` | HASH160 | `bb1be98c142444d7a56aa3981c3942a978e4dc33` |
| empty | HASH256 | `5df6e0e2761359d30a8275058e299fcc0381534545f55cf43e41983f5d4c9456` |
| tag=`TapTweak`, msg=`79be667e…f81798` | tagged_hash | `3cf5216d476a5e637bf0da674e50ddf55c403270dd36494dfcca438132fa30e7` |

HASH160 of G compressed (`0279be66…f81798`) is `751e76e8199196d454941c45d1b3a323f1433bd6` (BIP-173).

### Vector G — secret `1`

| Field | Value |
|---|---|
| secret hex | `0000000000000000000000000000000000000000000000000000000000000001` |
| WIF compressed main | `KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn` |
| WIF uncompressed main | `5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreAnchuDf` |
| WIF compressed test | `cMahea7zqjxrtgAbB7LSGbcQUr1uX1ojuat9jZodMN87JcbXMTcA` |
| WIF uncompressed test | `91avARGdfge8E4tZfYLoxeJ5sGBdNJQH4kvjJoQFacbgwmaKkrx` |
| pubkey compressed | `0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798` |
| pubkey uncompressed | `0479be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798483ada7726a3c4655da4fbfc0e1108a8fd17b448a68554199c47d08ffb10d4b8` |
| P2PKH compressed main | `1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH` |
| P2PKH uncompressed main | `1EHNa6Q4Jz2uvNExL497mE43ikXhwF6kZm` |
| P2PKH compressed test | `mrCDrCybB6J1vRfbwM5hemdJz73FwDBC8r` |
| P2PKH uncompressed test | `mtoKs9V381UAhUia3d7Vb9GNak8Qvmcsme` |
| P2WPKH main | `bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4` |
| P2WPKH test | `tb1qw508d6qejxtdg4y5r3zarvary0c5xw7kxpjzsx` |
| P2TR output x (tweaked) | `da4710964f7852695de2da025290e24af6d8c281de5a0b902b7135fd9fd74d21` |
| P2TR main | `bc1pmfr3p9j00pfxjh0zmgp99y8zftmd3s5pmedqhyptwy6lm87hf5sspknck9` |
| P2TR test | `tb1pmfr3p9j00pfxjh0zmgp99y8zftmd3s5pmedqhyptwy6lm87hf5ssk79hv2` |

### Vector Wiki — secret `18e14a7b6a307f426a94f8114701e7c8e774e7f9a47e2c2035db29a206321725`

| Field | Value |
|---|---|
| WIF compressed main | `Kx45GeUBSMPReYQwgXiKhG9FzNXrnCeutJp4yjTd5kKxCitadm3C` |
| WIF uncompressed main | `5J1F7GHadZG3sCCKHCwg8Jvys9xUbFsjLnGec4H125Ny1V9nR6V` |
| pubkey compressed | `0250863ad64a87ae8a2fe83c1af1a8403cb53f53e486d8511dad8a04887e5b2352` |
| P2PKH uncompressed (wiki) | `16UwLL9Risc3QfPqBUvKofHmBQ7wMtjvM` |
| P2PKH compressed | `1PMycacnJaSqwwJqjawXBErnLsZ7RkXUAs` |
| P2WPKH main | `bc1q7499s50fxu4c0qg23esvm5h8elvqkm33r2tdza` |
| P2TR main | `bc1p5ymjdxmqesnfeg42lyjh642570nxp7dgp0uzh22uq4z3gew9yymst6pshk` |

### Vector WIF-wiki — secret `0c28fca386c7a227600b2fe50b7cae11ec86d3bf1fbe471be89827e19d72aa1d`

| Field | Value |
|---|---|
| WIF uncompressed main | `5HueCGU8rMjxEXxiPuD5BDku4MkFqeZyd4dZ1jvhTVqvbTLvyTJ` |
| WIF compressed main | `KwdMAjGmerYanjeui5SHS7JkmpZvVipYvB2LJGU1ZxJwYvP98617` |
| pubkey compressed | `02d0de0aaeaefad02b8bdc8a01a1b8b11c696bd3d66a2c5f10780d95b7df42645c` |

### `--from text`

| Text | SHA-256 | WIF compressed main |
|---|---|---|
| `test01` | `678e82d907d3e6e71f81d5cf3ddacc3671dc618c38a1b7a9f9393a83d025b296` | `Kzh1d5pXSZLtwsgENakrfCjuGy9txPEb3aEb2y8yyZo65qDs8bTu` |
| `Secret Passphrase` | `76ce9bba9487266738e3c4f0b3cfa4be0c0eba52ed1c3c425e06900442efe5e1` | `L1Cf21MBhiZX9QFTAhN3PGJkyvQzN4CuHwhasHsdV9tkEfiiB8Ug` |

### BIP-341 wallet vector 0 (primary P2TR unit test)

| Field | Value |
|---|---|
| internal privkey | `6b973d88838f27366ed61c9ad6367663045cb456e28335c109e30717ae0c6baa` |
| internal x-only pubkey | `d6889cb081036e0faefa3a35157ad71086b123b2b144b649798b494c300a961d` |
| tweak | `b86e7be8f39bab32a6f2c0443abbc210f0edac0e2c53d501b36b64437d9c6c70` |
| tweaked output x | `53a1f6e454df1aa2776a2814a721372d6258050de330b3c6d10ee8f4e0dda343` |
| address | `bc1p2wsldez5mud2yam29q22wgfh9439spgduvct83k3pm50fcxa5dps59h4z5` |

`btk privkey --encoding wif` of that hex, piped to `btk address --type p2tr`,
must equal that address.

### Vector 6 — odd-Y compressed (`03`) P2TR

Secret `6`. A wrong `ec_pubkey_tweak_add` on the odd point fails this row and
can still pass Vector G / wallet vector 0.

| Field | Value |
|---|---|
| secret hex | `0000000000000000000000000000000000000000000000000000000000000006` |
| WIF compressed main | `KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU76Myig6zj` |
| pubkey compressed | `03fff97bd5755eeea420453a14355235d382f6472f8568a18b2f057a1460297556` |
| P2PKH compressed main | `1Cf2hs39Woi61YNkYGUAcohL2K2q4pawBq` |
| P2WPKH main | `bc1q0ldfeupqc9k2eaffep7cm6yml3ct3jwtwzqt7k` |
| P2TR main | `bc1p4rsld9ryjhte00drc0r23r8ngd63xrzh5s4fvmy6q5yt70xzlsdqcuvtzv` |

### Bech32 / Bech32m

Decode-only (uppercase HRP valid; encoder emits lowercase): `A12UEL5L` (bech32),
`A1LQFN3A` (bech32m). Encode: `a12uel5l`, `a1lqfn3a`,
`abcdef1qpzry9x8gf2tvdw0s3jn54khce6mua7lmqqqxw` (bech32, data `0..31`).

| Address | scriptPubKey |
|---|---|
| `BC1QW508D6QEJXTDG4Y5R3ZARVARY0C5XW7KV8F3T4` | `0014751e76e8199196d454941c45d1b3a323f1433bd6` |
| `bc1p0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7vqzk5jj0` | `512079be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798` |

Invalid: mixed case; bech32 checksum on a v1 address; bech32m checksum on a v0 address.

### Rejected scalars

32 zero bytes, hex `n`, and hex `n+1` → `private key out of range`.

### BIP-141 txid (handmade 1-in-1-out P2WPKH)

Version 2. One input: 32-byte zero prevout, vout 0, empty scriptSig,
`nSequence = 0xffffffff`. One output: 100000 sats, P2WPKH of G. Witness: 72-byte
dummy (`30` + 70 zero bytes + `01`) and G’s compressed pubkey. `nLockTime = 0`.

Non-witness (txid preimage):

```
020000000100000000000000000000000000000000000000000000000000000000000000000000000000ffffffff01a086010000000000160014751e76e8199196d454941c45d1b3a323f1433bd600000000
```

Full witness (wtxid preimage, 192 bytes). After the 22-byte output script comes
the per-input witness (`02` = two items, `48` = 72-byte dummy, `21` = 33-byte
pubkey). No extra count byte between `scriptPubKey` and the stack.

```
0200000000010100000000000000000000000000000000000000000000000000000000000000000000000000ffffffff01a086010000000000160014751e76e8199196d454941c45d1b3a323f1433bd60248300000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001210279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f8179800000000
```

| | Internal (HASH256, as stored) | Display (byte-reversed) |
|---|---|---|
| **txid** (use this) | `3c58a2ad2dfc2f132e6dd137844f6e6bd749e33672f5e24d5109cea990c282a8` | `a882c290a9ce09514de2f57236e349d76b6e4f8437d16d2e132ffc2dada2583c` |
| **wtxid** (do not use) | `dd22da01b8929076ae782656fb7dfd1d3fe12a39c895b38aa533dbaa7d0806a1` | `a106087daadb33a58ab395c8392ae13f1dfd7dfb562678ae769092b801da22dd` |

Assert txid ≠ wtxid. Round-trip-parse the 192-byte hex: one input, one output,
**two** witness items of length 72 and 33.

### CompactSize goldens

`0=00`, `1=01`, `23=17`, `127=7f`, `128=80`, `200=c8`, `252=fc`,
`253=fd fd 00`, `255=fd ff 00`, `256=fd 00 01`.

## Security

- Invalid scalar (`0`, `≥ n`) must not be accepted.
- Only libsecp256k1 multiplies points.
- No secrets on stderr or in argv traces.
- Config file `0600`, `~/.btk` `0700`.
- `config dump` / `get rpc.auth` always `********`.
- SHA-256(passphrase) is not a KDF — allowed on `privkey` only; README warns.
- CSPRNG short read is fatal.
- Balance: single writer thread, LevelDB write batches, BIP-141 txid, reorg
  check refuses incremental `--sync`.
- Threat model: a local CLI run by the operator. Not a daemon. Not multi-tenant.
- Observability: stderr for errors and balance progress. Never mix with NDJSON
  stdout. No log file.

## Conventions

- C++17, GNU Makefile, no Boost, no CMake. Unix only.
- Exceptions internally; `main` maps them to exit 1.
- Tests compare parsed JSON objects, not string tables. CLI tests spawn `bin/btk`.
- When adding a command: register it, add `SRC`, add `test/cli/test_<cmd>.py`
  (and unit tests if there is new core), pin `--help`, update man pages if the
  command has one, and update README.md + this file.
- Keep user-facing examples in README.md working. Prefer Vector G when a
  documented WIF/address is needed.
