# Bitcoin Toolkit 4.0.0 — Product Design (Greenfield)

| Field | Value |
|---|---|
| **Document** | Product spec + incremental implementation plan |
| **Author** | Bitcoin Toolkit maintainers |
| **Date** | 2026-08-14 |
| **Status** | Ready |
| **Target product** | Bitcoin Toolkit 4.0.0 |
| **Language** | C++17 (GNU Makefile, no Boost, no CMake) |
| **License** | GNU GPL v3 |

This document designs **version 4.0.0 as a new product**. It is not a port of 3.1.2 flags, man pages, tests, error strings, or vendored trees. After the wipe, an engineer implements from **this file alone**. Golden vectors live here, re-derived from Bitcoin consensus rules and published BIPs — not by reference to files that will be deleted.

---

## Overview

Bitcoin Toolkit (`btk`) is a Git-style Unix CLI for a small, composable set of Bitcoin jobs: create and convert private keys, derive public keys, derive addresses, handshake with a P2P peer, maintain a local address→satoshi index, and hold a few RPC/path defaults.

4.0.0 throws away the 3.1.2 implementation (homemade secp256k1, `BUFSIZ` buffers, process-global network, a bag-of-strings JSON array, a misnamed `--bech32m` that was not Taproot). The replacement is a C++17 single binary whose commands speak a **typed object stream** over stdin/stdout. Users compose tools with pipes:

```bash
btk privkey --new | btk pubkey | btk address --type p2wpkh
btk privkey --new --stream | btk address --type p2pkh --match '^1bri'
```

Each line of the default pipe is one JSON object with a `type` field. A `--plain` mode still exists for people who want a bare WIF or address per line. Address types are named after the script they actually produce: `p2pkh`, `p2wpkh`, and **real BIP-341 `p2tr`**. There is no `--bech32m` flag.

---

## Background & Motivation

3.1.2 proved the product shape: a single `btk` binary, Git-style subcommands, Unix pipes, optional Bitcoin Core RPC / chainstate for balances. It also accumulated accidents that users should not inherit:

| Accident | Why it is not 4.0.0 |
|---|---|
| Homemade EC + mini-gmp, no order check | Consensus-critical math belongs in `libsecp256k1`. A key of `0` or `≥ n` is not a key. |
| JSON array of untyped strings | `["L5…"]` and `["02…"]` look the same. A downstream command has to guess. |
| `--bech32m` = HASH160 + witness v1 | That is not BIP-341. 4.0.0 will not ship a flag that lies about Taproot. |
| Process-global `--testnet` leftover | A mixed list must not infect later items. Network is per object. |
| Balance index hashed the witness serialization (wtxid) as if it were txid | SegWit spends then miss. 4.0.0 uses BIP-141 txid. |
| `btk help` shells out to `man` | Help must work on a box without man pages. |
| Short-opt soup (`-W -X -D -C -U -Q -R`) | Hard to remember, hard to compose. Long options with a tiny, consistent vocabulary. |
| Silent “string → SHA256 → key” guess | A typo’d WIF became a passphrase key. Explicit `--from-text` only. |

The rebuild is the chance to keep the *jobs* and replace the *contract*.

---

## Goals & Non-Goals

### Goals

- Git-style invocation: `btk <command> [options] [input…]`.
- Unix pipes as the composition mechanism. Default wire format is **typed NDJSON**.
- High-level capabilities: private keys, public keys, addresses (P2PKH / P2WPKH / BIP-341 P2TR), P2P node handshake, embedded help, version, local address-balance index, optional config.
- Vanity / search as a usage pattern: stream keys, filter addresses with `--match`.
- C++17, GNU Makefile, no Boost, no CMake/Conan/vcpkg as a required user step.
- Extremely low dependency count. One required package: `libsecp256k1`. LevelDB optional (balance only).
- Default automated tests are offline.
- An engineer can implement Phase 1 (private keys + the pipe runtime) from this file the day after the wipe.

### Non-Goals

| Item | Reason |
|---|---|
| QR terminal output | Cute, not load-bearing. `btk address --plain \| qrencode -t ANSIUTF8` covers it. No Nayuki tree. |
| Decimal private-key encoding | Huge integers, no wallet uses them. Hex and WIF are enough. |
| Raw binary stdin/stdout (`-R` / `-B`) | Hex in a typed object is the binary escape hatch. No unframed 32-byte dumps in a pipe. |
| Hidden `sbd` input type | Undocumented, not hashed, easy to misuse. |
| Silent passphrase guessing | `--from-text` is the explicit replacement. |
| `--rehash` / vanity-from-incrementing-hash | Stream new CSPRNG keys instead. |
| HD / BIP-32 / BIP-39 / PSBT / message sign / tx build | Never user-facing; still out of scope. |
| IPv6 P2P, signet, regtest | 4.0.0 node is IPv4 mainnet. `--network` exists for keys/addresses; node ignores it. |
| P2SH / P2WSH *generation* | Address command is key-path only. The balance indexer *does* recognize those scripts when they appear on-chain. |
| Cloning 3.1.2 flags, man pages, tests, or `src/mods/{cJSON,QRCodeGen}` | The wipe deletes them. Do not copy them back. |
| Windows | Unix-only (`getentropy`/`/dev/urandom`, `$HOME`, Makefile install). |
| CMake / Conan / vcpkg as required | Makefile only. |
| Compatibility with 3.1.2 scripts | 4.0.0 is a new CLI. Document a short migration note in the README; do not emulate. |

---

## Key Decisions

| # | Decision | Rationale |
|---|---|---|
| D1 | **Language: C++17, no Boost** | RAII for sockets/files/LevelDB; `std::string`/`std::vector` kill `BUFSIZ`; gcc 7+ / clang 5+. |
| D2 | **Required package: `libsecp256k1`** | Security-critical. Distro-patched. `apt install libsecp256k1-dev` / `dnf install libsecp256k1-devel` / `brew install secp256k1`. No homemade EC. |
| D3 | **Hashes: implement SHA-256 and RIPEMD-160 in-tree** | Avoids OpenSSL 3 legacy-provider hell. ~300 lines, tested against FIPS/Wikipedia vectors below. Not copied from 3.1.2. |
| D4 | **JSON: vendor picojson (fresh download)** | Header-only, BSD-2-Clause, small. See pin in *Third-party pins*. Do **not** copy `src/mods/cJSON`. Parses one complete value: NDJSON/object streams are incremental; a JSON **array** is parsed whole, then walked (Issue: picojson is not a streaming array parser). |
| D5 | **QR: drop** | See Non-Goals. |
| D6 | **Command names stay `privkey` / `pubkey` / `address` / `node` / `help` / `version` / `balance` / `config`** | They are the domain nouns. Inventing `key`/`addr`/`peer` saves no typing and breaks muscle memory of the *jobs* without improving them. The option language is what changes. |
| D7 | **Long options, tiny vocabulary** | `--network`, `--out`, `--type`, `--new`, `--match`, `--stream`. A handful of shorts (`-h -V -n -o`). No `-W -X -D -Q -R`. |
| D8 | **Default pipe = NDJSON typed objects** | One JSON object per line, `type` discriminator, `fflush` after each object when streaming. Pretty JSON and `--plain` are opt-in. |
| D9 | **Network is per object, never process-global** | WIF version byte sets that item’s network. **Both `privkey` and `pubkey` objects carry `network`.** `--network` applies to generated keys and to hex inputs. Address uses WIF → object `network` → flag → mainnet (never walks `source`). |
| D10 | **Private keys must be in `[1, n-1]`** | `secp256k1_ec_seckey_verify`. Reject 0 and `≥ n`. |
| D11 | **Address `--type p2pkh\|p2wpkh\|p2tr`** | Names the script. Default `p2wpkh` (modern, cheap, universally received). **No `--bech32m` flag.** `p2tr` is BIP-341 key-path, empty script tree. |
| D12 | **Vanity is `--stream` + `--match`** | `btk privkey --new --stream \| btk address --type p2pkh --match '^1bri'`. Matching address objects include `source` (the privkey). |
| D13 | **`--from-text` / `--from-file` are explicit** | SHA-256 of the bytes → key. Never the default parse path. |
| D14 | **Help is embedded** | `btk help`, `btk help <cmd>`, `btk --help`, `btk <cmd> --help`. Newly written man pages are optional install artifacts; help does not call `man`. |
| D15 | **Version 4.0.0, GPL-3** | Greenfield break. New tree is GPL-3 throughout (match existing `LICENSE`). |
| D16 | **`btk node` is IPv4 mainnet, port 8333, 15 s timeout** | Cheap `--network` on node would also need magic bytes + default port; defer. Keys/addresses still take `--network`. |
| D17 | **Balance store is a new LevelDB layout** | Address → uint64 LE sats; outpoint → address+amount; metadata for tip/height. BIP-141 txid. Single writer. Progress on stderr. 3.1.2 DBs are not readable. |
| D18 | **Config dump redacts `rpc.auth` as `********`** | Exactly eight asterisks. File mode `0600`. |
| D19 | **Exceptions internally, exit codes at `main`** | `BtkError` with a public message. `main` prints `btk <command>: <message>` on stderr and returns 1. No secrets in messages. |
| D20 | **Default `make test` is offline** | Live P2P only under `BTK_RUN_NET=1` / `make test-net`. |
| D21 | **`--new` not `--create`** | Reads as “make a key”. Used on `privkey` (CSPRNG) and `balance` (build index) is *not* overloaded: balance uses `--build`. |
| D22 | **TTY does not change the contract** | Default stdout is always NDJSON. No hidden pretty-print when isatty. `--out json` is the human pretty form. |

---

## Proposed Design

### Architecture

`btk` is one short-lived process. One command per invocation. Composition is the Unix pipe.

```mermaid
flowchart TB
    subgraph cli [CLI]
        MAIN[src/main.cpp]
        DISP[cli/dispatcher]
        OPTS[cli/options]
        IO[cli/io]
        OUT[cli/output]
    end

    subgraph cmds [Commands]
        PK[cmd/privkey]
        PUB[cmd/pubkey]
        ADDR[cmd/address]
        NODE[cmd/node]
        HELP[cmd/help]
        VER[cmd/version]
        BAL[cmd/balance]
        CFG[cmd/config]
    end

    subgraph core [Core]
        KEY[core/key]
        ENC[core/encoding]
        HASH[core/hash]
        RAND[core/random]
        JSON[core/json_io]
        NETW[core/network]
    end

    subgraph net [Network]
        P2P[net/p2p]
        RPC[net/jsonrpc]
    end

    subgraph chain [Chain]
        TX[chain/tx]
        BLK[chain/block]
        SCR[chain/script]
        CS[chain/chainstate]
        BDB[chain/balance_db]
    end

    MAIN --> DISP
    DISP --> OPTS
    DISP --> IO
    DISP --> OUT
    DISP --> cmds
    PK --> KEY
    PK --> ENC
    PK --> RAND
    PUB --> KEY
    ADDR --> KEY
    ADDR --> ENC
    NODE --> P2P
    BAL --> BDB
    BAL --> CS
    BAL --> RPC
    KEY --> SECP[libsecp256k1]
    HASH --> SHA[src/core/sha256 + ripemd160]
    JSON --> PICO[third_party/picojson]
    BDB --> LDB[libleveldb]
    CFG --> UTIL[util/config]
```

```mermaid
sequenceDiagram
    participant U as User / pipe
    participant M as main
    participant D as Dispatcher
    participant C as Command
    participant O as stdout

    U->>M: argv + optional stdin
    M->>D: parse command + options
    alt parse failed or unknown command
        D-->>U: stderr + exit 1 (no config mkdir)
    else help/version flags
        D-->>O: embedded text / version object
    else command
        D->>D: load config only for config / balance (file must already exist)
        alt generator (--new / --from-text / --from-file / node / version / help / config / balance --build|--update)
            loop once, --count times, or forever if --stream
                D->>C: run()
                D->>O: write object, flush if stream or ndjson
            end
        else transformer
            loop each argv item or each stdin object/line
                D->>C: run(item)
                D->>O: write immediately (always incremental)
            end
        end
    end
```

**Flush rules (this is the whole streaming contract):**

1. Transformers are **always incremental**. One input item → zero or more output objects, written before the next item is read.
2. `--out ndjson` (default) and `--out plain`: `fflush` after every object / line.
3. `--out json`: buffer into one JSON value (object if 1, array if N) and print at the end, **unless** `--stream` is set, in which case behave as ndjson.
4. Generators emit without reading the object stream. `privkey --new` emits `--count` items (default 1). `--stream` alone is **infinite** until SIGINT. `--stream --count N` is finite N. `privkey --from-text` / `--from-file` emit exactly one key (or error).
5. Empty stdin and no positional items on a **transformer**: print nothing, exit 0.
6. A producer that writes one NDJSON object and sleeps must cause a consumer that is reading an **object stream** (`--in auto` seeing `{`, or `--in ndjson`) to emit before the producer exits. That is the vanity flush test. A pretty JSON **array** (`[…]`) may be parsed as one picojson value and then emitted item-by-item; it is **not** required to yield elements before the producer closes the array.

### Directory layout

```text
bitcoin-toolkit/
├── Makefile
├── LICENSE                          # existing GPL-3 file, kept across the wipe
├── README.md                        # newly written
├── REBUILD.md                       # THIS document (replaces the rejected 3.1.2-compat spec)
├── man/                             # newly written; optional
│   ├── btk.1
│   ├── btk-privkey.1
│   ├── btk-pubkey.1
│   ├── btk-address.1
│   ├── btk-node.1
│   ├── btk-help.1
│   ├── btk-version.1
│   ├── btk-balance.1
│   └── btk-config.1
├── src/
│   ├── main.cpp
│   ├── version.hpp                  # BTK_VERSION_{MAJOR,MINOR,PATCH} = 4,0,0
│   ├── cli/
│   │   ├── dispatcher.cpp / .hpp
│   │   ├── options.cpp / .hpp
│   │   ├── io.cpp / .hpp
│   │   └── output.cpp / .hpp
│   ├── cmd/
│   │   ├── command.hpp
│   │   ├── privkey.cpp
│   │   ├── pubkey.cpp
│   │   ├── address.cpp
│   │   ├── node.cpp
│   │   ├── help.cpp
│   │   ├── version.cpp
│   │   ├── balance.cpp
│   │   └── config.cpp
│   ├── core/
│   │   ├── hash.cpp / .hpp          # SHA256, RIPEMD160, HASH256, HASH160, tagged_hash
│   │   ├── random.cpp / .hpp
│   │   ├── hex.cpp / .hpp
│   │   ├── base58.cpp / .hpp
│   │   ├── bech32.cpp / .hpp        # bech32 + bech32m
│   │   ├── json_io.cpp / .hpp       # picojson wrap + NDJSON reader
│   │   ├── privkey.cpp / .hpp
│   │   ├── pubkey.cpp / .hpp
│   │   ├── address.cpp / .hpp
│   │   ├── secp.cpp / .hpp          # RAII secp256k1_context
│   │   └── network.hpp              # enum class Network { Main, Test }
│   ├── net/
│   │   ├── p2p.cpp / .hpp
│   │   └── jsonrpc.cpp / .hpp
│   ├── chain/
│   │   ├── compactsize.cpp / .hpp
│   │   ├── varint.cpp / .hpp        # Bitcoin Core varint (chainstate)
│   │   ├── script.cpp / .hpp
│   │   ├── transaction.cpp / .hpp
│   │   ├── block.cpp / .hpp
│   │   ├── chainstate.cpp / .hpp
│   │   └── balance_db.cpp / .hpp
│   └── util/
│       ├── error.cpp / .hpp
│       └── config.cpp / .hpp
├── third_party/
│   ├── picojson/
│   │   ├── picojson.h
│   │   └── NOTICE                    # URL, commit, license quote
│   └── README.md                     # how the pins were fetched
└── test/
    ├── runner.py                     # new
    ├── cli/                          # new Python integration, one module per command
    ├── unit/                         # new C++ unit tests, no gtest
    └── fixtures/                     # created during implementation, not copied
```

No `ctrl_mods` / `mods` split. Commands depend on libraries; libraries do not know about CLI flags.

### Third-party pins (fresh vendors — do not copy 3.1.2)

| Library | Role | License | Fetch |
|---|---|---|---|
| **picojson** | JSON parse/serialize | BSD-2-Clause | `https://raw.githubusercontent.com/kazuho/picojson/111c9be5188f7350c2eac9ddaedd8cca3d7bf394/picojson.h` |
| **libsecp256k1** | EC, tweak | MIT | distro package, not vendored |
| **LevelDB** | balance DB | BSD-3-Clause | distro package, optional |

picojson NOTICE (write this file at fetch time):

```text
picojson.h
Source : https://github.com/kazuho/picojson
Commit : 111c9be5188f7350c2eac9ddaedd8cca3d7bf394  (2021-01-17)
License: BSD-2-Clause
URL    : https://raw.githubusercontent.com/kazuho/picojson/111c9be5188f7350c2eac9ddaedd8cca3d7bf394/picojson.h
```

SHA-256 and RIPEMD-160 are **implemented in `src/core/`** from FIPS 180-4 and the RIPEMD-160 spec, with the golden hashes in *Appendix A*. They are not vendored from 3.1.2 and not taken from OpenSSL.

### Build system

GNU Make, single binary `bin/btk`.

```make
CXX      ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -I src -I third_party
LIBS     ?= -lsecp256k1 -lpthread

# Probe libleveldb. If missing, -DBTK_NO_LEVELDB and do not link it.
# libsecp256k1 is required: probe fails → error with the apt/dnf/brew line.
```

Targets: `all`, `test` (unit + offline CLI), `test-unit`, `test-cli`, `test-net`, `install`, `uninstall`, `clean`.

`prefix ?= /usr/local`. Install `bin/btk` and, if present, `man/btk*.1`.

`libsecp256k1` missing:

```text
error: libsecp256k1 is required
  Debian/Ubuntu : sudo apt-get install libsecp256k1-dev
  Fedora        : sudo dnf install libsecp256k1-devel
  macOS         : brew install secp256k1
```

### Command dispatcher

```cpp
// src/cmd/command.hpp
#pragma once
#include <optional>
#include <string>
#include <vector>
#include "cli/options.hpp"
#include "core/json_io.hpp"

class Command {
public:
    virtual ~Command() = default;
    virtual const char* name() const = 0;
    virtual const char* summary() const = 0;
    virtual void register_options(OptionSpec&) const = 0;
    virtual bool is_generator(const Options&) const = 0;
    virtual void init(Options&) {}
    // Transformer: one input object (or bare string wrapped as an object).
    // Generator: called with std::nullopt.
    virtual std::vector<JsonObject> run(const Options&,
                                        const std::optional<JsonObject>&) = 0;
};
```

`main` looks up `argv[1]`. Global flags may appear **before** the command (`btk --config PATH privkey --new`) or after, except `--help`/`--version` which also work with no command.

Unknown command, stderr, exit 1:

```text
btk: unknown command 'foo'
See 'btk help' for a list of commands.
```

No arguments at all: print the overview help on **stderr**, exit 1 (same idea as git).

`btk help` / `btk --help`: overview on **stdout**, exit 0.

Do not create `~/.btk` on unknown command or failed option parse.

**When config is loaded.** After a successful parse, load `~/.btk/config.json` (or `--config` / `$BTK_CONFIG`) **only** for `config` and `balance`. Phases 1–6 (`privkey`, `pubkey`, `address`, `node`, `help`, `version`) never open the config file, so a corrupt file cannot break key generation. If the file is missing, those two commands use compiled defaults (no mkdir). If the file exists but is invalid JSON or has a wrong type for a known key: `btk <command>: invalid config file`. Unknown JSON fields on disk are ignored.

**Resolving the config path.** `--config PATH` wins, else `$BTK_CONFIG`, else `$HOME/.btk/config.json`. `--config` and `$BTK_CONFIG` may be relative (cwd). If the resolved path needs `$HOME` and `HOME` is unset: `HOME is not set`. `rpc.auth` is `user:pass` split on the **first** colon (`user` may not contain `:`; `pass` may). No Bitcoin Core cookie file in 4.0.0.

### Option language

One vocabulary across commands.

| Short | Long | Arg | Meaning |
|---|---|---|---|
| `-h` | `--help` | | Print this command’s help, exit 0. |
| `-V` | `--version` | | Print a `version` object, exit 0. |
| | `--config` | path | Config file. Default: `$BTK_CONFIG` if set, else `~/.btk/config.json`. |
| `-n` | `--network` | `mainnet`\|`testnet` | Network for *this* invocation’s generated / hex items. |
| `-o` | `--out` | `ndjson`\|`json`\|`plain` | Output framing. Default `ndjson`. |
| | `--in` | `auto`\|`ndjson`\|`json`\|`plain` | Stdin framing. Default `auto`. |
| `-s` | `--stream` | | Generator: emit until SIGINT. Combined with `--count N`, emit exactly N (finite). Forces per-item flush. |
| `-c` | `--count` | N | Generator: emit N items. Default 1. N must be a positive integer (`≥ 1`). |

Per-command additions are listed below. Repeatable flags are called out. Unknown flags are errors.

`getopt_long` is fine. Prefix `--` stops option parsing. Warn in help that `POSIXLY_CORRECT` stops parsing at the first non-option.

### Typed pipe format

This is the load-bearing contract.

#### Envelope

Every default-mode record is one JSON **object**. Required field:

| Field | Type | Meaning |
|---|---|---|
| `type` | string | `privkey` \| `pubkey` \| `address` \| `node` \| `balance` \| `version` \| `config` |

Unknown `type` on stdin: error, exit 1 (do not guess) **except** where a command lists a type it accepts (below). Extra fields are ignored on input (forward compatible) and must not be invented on output.

**JSON integers.** `sats`, `port`, `height`, `protocol`, `timestamp`, and `rpc.port` are serialized as decimal digit strings with no exponent and no fraction (`5000000000`, never `5e+09`). picojson parse accepts ordinary JSON numbers; we write with a dedicated integer printer, not `stringstream` defaults.

#### `privkey`

```json
{
  "type": "privkey",
  "encoding": "wif",
  "network": "mainnet",
  "compressed": true,
  "data": "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn"
}
```

| Field | Values |
|---|---|
| `encoding` | `wif` (default on output) or `hex` |
| `network` | `mainnet` \| `testnet` |
| `compressed` | bool. WIF compression flag / “prefer compressed pubkey”. |
| `data` | WIF string, or 64 lowercase hex chars (the 32-byte scalar). Hex never includes a trailing `01`/`00` flag — compression is the boolean. |

#### `pubkey`

```json
{
  "type": "pubkey",
  "encoding": "hex",
  "network": "mainnet",
  "compressed": true,
  "data": "0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798"
}
```

`data` is 66-char (`02`/`03`) or 130-char (`04`) lowercase hex. `network` is copied from the parent privkey, or from `--network`, or `mainnet` for a bare hex pubkey. Optional `source` (a `privkey` object) when the input was a privkey. Address must **not** walk `source` for network (one level only).

#### `address`

```json
{
  "type": "address",
  "style": "p2wpkh",
  "network": "mainnet",
  "data": "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4"
}
```

`style`: `p2pkh` \| `p2wpkh` \| `p2tr` \| `p2sh` \| `p2wsh`. The last two appear on **balance** output only (we index them; we do not generate them).

`--match` filters on `data`. Optional `source` is the immediate parent object (privkey or pubkey).

#### `node`

```json
{
  "type": "node",
  "host": "seed.bitcoin.sipa.be",
  "ip": "1.2.3.4",
  "port": 8333,
  "protocol": 70016,
  "user_agent": "/Satoshi:24.0.1/",
  "height": 786299,
  "services": ["NODE_NETWORK", "NODE_WITNESS", "NODE_NETWORK_LIMITED"],
  "relay": true,
  "timestamp": 1682030946
}
```

#### `balance`

```json
{
  "type": "balance",
  "address": "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa",
  "sats": 0
}
```

`sats` is a JSON number (uint64, decimal integer — see above). Unknown address → `0`, not an error.

Query input (argv or stdin), in order:

| Input | Action |
|---|---|
| object `type=address` | query `data` |
| object `type=balance` | re-query `address` |
| object any other `type` | error `expected an address` |
| bare string | parse as a Bitcoin address |

#### `version`

```json
{
  "type": "version",
  "version": "4.0.0",
  "secp256k1": true,
  "leveldb": true
}
```

`leveldb` is whether this binary was linked with LevelDB.

#### `config`

```json
{
  "type": "config",
  "rpc.host": "127.0.0.1",
  "rpc.port": 8332,
  "rpc.auth": "********",
  "balance.path": "/home/alice/.btk/balance",
  "chainstate.path": "/home/alice/.bitcoin/chainstate"
}
```

`rpc.auth` is **always** eight asterisks when present. If unset, the key is omitted.

#### `--out` / `--in`

| Mode | Output | Input |
|---|---|---|
| `ndjson` (out default) | One object, minified, one line, trailing `\n`. | One object per line. Blank lines skipped. |
| `json` | Pretty-printed (2-space) single object, or a pretty array of objects. Trailing `\n`. | One complete picojson value: a single object, or an array of objects/strings. An array is parsed **as a whole** (picojson is not a streaming array parser), then emitted item-by-item. Concatenated top-level objects (`{}{}`) are not required under `--in json`; use `ndjson` / `auto` for that. |
| `plain` | The primary string only, one per line (table below). | One bare string per line. |
| `auto` (in default) | — | Skip leading whitespace. `{` → object stream (read one complete object at a time — vanity). `[` → one JSON array, then walk elements. Else `plain`. |

**`--out plain` primary string**

| Command | Primary |
|---|---|
| `privkey` | `data` (WIF or hex) |
| `pubkey` | `data` (hex) |
| `address` | `data` (address) |
| `balance` | `sats` as decimal digits |
| `version` | `version` (`4.0.0`) |
| `node` | `ip:port` (example `1.2.3.4:8333`) |
| `config dump` | one `key=value` per line, dotted keys, `rpc.auth=********` |
| `config get` | the raw value only (redacted if `rpc.auth`) |
| `help` | not JSON; `--out` is ignored |

Positional argv items: if an argument starts with `{`, parse as a JSON object; else treat as a bare string.

**Bare-string interpretation** (when the command did not get a typed object):

| Command | Guess order | Failure |
|---|---|---|
| `privkey` | WIF (base58check, version `0x80`/`0xEF`) → 64-char hex → error | Do **not** SHA-256 the string. |
| `pubkey` | WIF → 66/130-char hex pubkey → 64-char hex privkey → error | |
| `address` | WIF → 66/130-char hex pubkey → 64-char hex **privkey** → error | 64-hex is **always a secret**, never an x-only internal key. |
| `balance` | A Bitcoin address (Base58Check or bech32/bech32m) | error |

**64-hex on `address` is a secret.** P2TR internal keys are also 32 bytes. `btk address --type p2tr <64 hex chars>` treats the string as a private key (`k·G`, then tweak). It never treats the bytes as an x-only public key. There is no `--from-xonly` in 4.0.0. Feed an x-only key only as a `pubkey` object with 66-char `02`/`03` (or 130-char `04`) `data`. Negative CLI test: the 64-hex of G’s x-coordinate `79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798` is a valid scalar (`< n`) and must **not** produce G’s P2TR `bc1pmfr3p9j00pfxjh0zmgp99y8zftmd3s5pmedqhyptwy6lm87hf5sspknck9` (it produces `int(x)·G` tweaked). Use secret-1 hex `00…01` for G’s address.

`--plain` on the producer + `--in plain` (or auto) on the consumer still composes:

```bash
btk privkey --new --out plain | btk address --type p2tr --out plain
```

#### Provenance (`source`)

When a transformer’s input is a typed object, the output includes `source` set to **that input object** (one level, not a deep chain). `--no-source` strips it.

`--source` / `--no-source` exist only on `address` (unknown flag elsewhere). Default: include `source` when the input was a typed object; omit it for bare strings. `--source` on a bare string synthesizes:

```json
{"type":"privkey","encoding":"wif","network":"mainnet","compressed":true,"data":"<the-bare-string>"}
```

or `encoding=hex` if the bare string was 64/66/130 hex; or `{"type":"pubkey",…}` if it was a hex pubkey. The synthetic `data` is the bare input; do not re-encode. `--no-source` always wins over `--source`.

Vanity relies on this:

```bash
btk privkey --new --stream | btk address --type p2pkh --match '^1bri'
```

```json
{"type":"address","style":"p2pkh","network":"mainnet","data":"1BRi…","source":{"type":"privkey","encoding":"wif","network":"mainnet","compressed":true,"data":"L3Uq…"}}
```

Pipe `privkey | pubkey | address` if you want the pubkey as `source` (not spendable from the address object alone). Vanity should be `privkey | address`.

### Error handling

| Code | When |
|---|---|
| 0 | Success, including “`--match` filtered everything” (empty stdout). |
| 1 | Any error. |
| 130 | SIGINT on a `--stream` generator (optional; if the runtime reports 130, fine; if it reports 0 after a clean unwind, also fine — tests must not require 130). |

Stderr, one line preferred:

```text
btk privkey: invalid WIF checksum
btk address: uncompressed key cannot produce p2wpkh or p2tr
btk node: connect timed out after 15s
```

Rules:

- Prefix is `btk <command>:` — never positional inputs (they may be secrets).
- No WIF, hex keys, passphrases, or `rpc.auth` values in messages.
- Validation wording is specified per command below so tests can assert it.

RAII owners: `SecpContext`, `Socket`, `LevelDb`, `LevelIter`, `LevelBatch`, `File`. Destructors run on exception unwind.

---

## Commands

### 1. `btk privkey` — Phase 1

Create a private key from the CSPRNG, convert encodings, or derive a key from explicit bytes.

```text
btk privkey --new [--count N] [--stream]
            [--encoding wif|hex] [--network mainnet|testnet]
            [--compressed | --uncompressed]
btk privkey [--encoding wif|hex] [--network mainnet|testnet]
            [--compressed | --uncompressed]
            [--from-text <str> | --from-file <path>]
            [item…]
```

| Long | Notes |
|---|---|
| `--new` | 32 CSPRNG bytes, then `secp256k1_ec_seckey_verify`; retry on failure (p ≈ 2⁻¹²⁸). Default compressed, mainnet, WIF. **Generator:** ignores positionals and does not read the object stream. |
| `--encoding` | Output encoding. Default `wif`. |
| `--compressed` / `--uncompressed` | Set the compression flag. If **both**, emit two objects (compressed first). Default compressed. |
| `--from-text STR` | **Generator.** `SHA256(UTF-8 bytes of STR)` → one key. Ignores positionals. Does not read the object stream. `-` is not magic (it is the literal string `-`). |
| `--from-file PATH` | **Generator.** `SHA256(entire file contents)` → one key. Ignores positionals. PATH `-` consumes stdin as raw bytes and is exclusive with piped objects. Any other PATH does not read stdin. |
| `--count N` | Only with `--new`. Emit N keys. Default 1. N must be `≥ 1`. |
| `--stream` | Only with `--new`. Alone: emit until SIGINT. With `--count N`: emit N, then stop. |

`--new`, `--from-text`, and `--from-file` are mutually exclusive (any two: `cannot combine --new, --from-text, and --from-file`).

Without `--new` / `--from-*`, this command is a **transformer**: each argv item or stdin object/line is parsed as WIF or hex (see guess order). `--encoding` selects the output encoding. `--network` re-encodes WIF to that network (hex items pick up `--network`, default mainnet). `--compressed` / `--uncompressed` flip the flag.

**Illegal combinations**

| Combo | Error |
|---|---|
| `--new` + `--from-text` / `--from-file` | `cannot combine --new, --from-text, and --from-file` |
| `--from-text` + `--from-file` | same |
| `--count` without `--new` | `--count requires --new` |
| `--stream` without `--new` | `--stream requires --new` |
| `--count 0` or negative or non-integer | `invalid --count` |
| `--from-file -` and positional items | `--from-file - cannot be combined with positional items` |

**Reject:** scalar `0` or `≥ n`. Message: `private key out of range`. Bad WIF checksum: `invalid WIF checksum`. Bad hex length: `invalid hex private key`.

CSPRNG: `getentropy(buf, 32)` if available (`<sys/random.h>`), else read exactly 32 bytes from `/dev/urandom`. Short read is fatal: `could not read CSPRNG`.

WIF:

- Payload: `version || 32-byte scalar || [0x01 if compressed]`.
- Version: mainnet `0x80`, testnet `0xEF`.
- Checksum: first 4 bytes of HASH256(payload).
- Base58. Leading `5` / `K`/`L` (main) and `9` / `c` (test) fall out of the version byte; do not pattern-match the first character as the decoder.

Hex output is lowercase. Hex input is case-insensitive.

`--from-text` / `--from-file` result is always treated as a compressed mainnet key unless flags say otherwise.

### 2. `btk pubkey` — Phase 2

Derive a public key, or recompress one.

```text
btk pubkey [--compressed | --uncompressed] [item…]
```

Input: `privkey` object, `pubkey` object, WIF, hex privkey, or hex pubkey.

- From a secret: `secp256k1_ec_pubkey_create` + `secp256k1_ec_pubkey_serialize`.
- From a pubkey: `secp256k1_ec_pubkey_parse` + serialize in the requested form.

Default compression: follow the input’s `compressed` field / WIF flag / existing prefix; if none, compressed. `--compressed` / `--uncompressed` override. Both flags → two objects.

Output `encoding` is always `hex`. `network` on the output object: from a typed input’s `network`; else from a WIF version byte; else `--network`; else `mainnet`.

Message on bad input: `not a private or public key`. `--match` is an unknown flag here.

### 3. `btk address` — Phase 3

```text
btk address [--type p2pkh|p2wpkh|p2tr]...
            [--network mainnet|testnet]
            [--match REGEX] [--ignore-case]
            [--source | --no-source]
            [item…]
```

| Long | Notes |
|---|---|
| `--type` | Repeatable. Default: one `p2wpkh`. Order of emission = order of flags. Unknown style: `unknown address type`. |
| `--match` | POSIX ERE on the address `data`. Inclusive. Once. **Address only** (unknown flag on every other command). |
| `--ignore-case` | Adds `REG_ICASE`. Address only. |
| `--source` | Force a `source` object even for bare-string input (see Provenance). |
| `--no-source` | Strip `source`. Wins over `--source`. |

`--match` compilation: `regcomp(pattern, REG_EXTENDED | REG_NOSUB [| REG_ICASE])` with the C locale in effect (`setlocale(LC_CTYPE, "C")` and `LC_COLLATE` C before `regcomp`, or `newlocale`/`uselocale` so `LC_COLLATE` cannot change `^1bri`). Invalid pattern: exit 1, `invalid match pattern`. `regexec` uses the same locale.

**Scripts**

| `--type` | Construction | HRP / version |
|---|---|---|
| `p2pkh` | Base58Check(`ver \|\| HASH160(serialized pubkey)`). `ver` = `0x00` main / `0x6F` test. Pubkey serialization follows the item’s compressed flag. | `1…` / `m…`/`n…` |
| `p2wpkh` | Bech32 (BIP-173), witness v0, 20-byte HASH160(**compressed** pubkey). | `bc` / `tb` |
| `p2tr` | Bech32m (BIP-350), witness v1, 32-byte **tweaked** x-only output key. BIP-341 key-path, empty script tree. | `bc` / `tb` |

Uncompressed key + `p2wpkh` or `p2tr`: error `uncompressed key cannot produce p2wpkh or p2tr`. (P2PKH of an uncompressed key is allowed.)

No `--bech32m` name. No HASH160-as-v1 path.

**BIP-341 `p2tr` algorithm (empty script tree):**

1. Require a compressed (or compressible) pubkey. Take `X` = the 32-byte x-coordinate. The `02`/`03` prefix is ignored except to parse `X`. Same `X` → same address (`lift_x` uses even Y, BIP-340).
2. Internal key `P = lift_x(X)`.
3. `t = int(tagged_hash("TapTweak", X))` where `tagged_hash(tag, msg) = SHA256(SHA256(tag) \|\| SHA256(tag) \|\| msg)`. Empty Merkle root: do **not** append a script-root. If `t ≥ n`, error `taproot tweak out of range`.
4. `Q = P + t·G`. Use `secp256k1_xonly_pubkey_parse` + `secp256k1_xonly_pubkey_tweak_add`, or `secp256k1_keypair_xonly_tweak_add` from a secret.
5. Program = `x(Q)` (32 bytes). Encode witness version 1 with **bech32m** (checksum xor constant `0x2bc830a3`). HRP from the item’s network.
6. A `p2tr` address is 62 characters (`bc1p` + 58). It is **not** a 42-character HASH160 bech32m string.

A compressed `03` key (odd Y) must produce the same address as the `02` key with the same X. Appendix A.2b is the odd-Y golden; A.2 / A.6 are even-Y.

**Network on `address`**, first match wins — do **not** walk `source`:

1. Input is WIF (bare or `privkey` with `encoding=wif`) → version byte.
2. Typed object has `network` → that value (`privkey` or `pubkey`).
3. `--network` flag.
4. `mainnet`.

**64-hex is a secret.** Never an x-only internal key. See Bare-string interpretation.

**Illegal combinations:** `--match` more than once → `cannot pass --match more than once`. `--stream` is accepted as “flush each output” (no-op; transformers already flush). `--count` is unknown.

### 4. `btk node` — Phase 4

```text
btk node <host[:port]>
btk node --host <host> [--port 8333]
```

IPv4 TCP only. Default port 8333. DNS via `getaddrinfo` (`AF_INET`). 15 second `SO_SNDTIMEO` / `SO_RCVTIMEO` (and/or `poll`) on connect and read.

**Host selection:** exactly one of positional `<host[:port]>` or `--host`. Both → `specify a host as a positional or --host, not both`. Neither → `missing host`. If the positional contains `:`, the suffix is the port (IPv4 only, so one colon). `--port` plus a positional that already has `:port` → `port specified twice`.

Handshake: send `version`, read the peer’s `version`, print the object, close. Do **not** send `verack`. One shot; `--stream` is an error (`node does not stream`).

`--out plain` prints `ip:port`. `--verbose` adds `raw` with `addr_recv`, `addr_trans`, `nonce`, `services_bits`.

**`version` payload layout** (all multi-byte integers little-endian **except** `addr_*` ports):

| Offset | Size | Field | We send |
|---|---|---|---|
| 0 | 4 | `version` int32 LE | `70015` (`7f 11 01 00`) |
| 4 | 8 | `services` uint64 LE | `0` |
| 12 | 8 | `timestamp` int64 LE | `time(NULL)` at runtime; tests use the frozen hex below |
| 20 | 8 | `addr_recv.services` uint64 LE | `0` |
| 28 | 16 | `addr_recv.ip` | IPv4-mapped `127.0.0.1` = `00 00 00 00 00 00 00 00 00 00 ff ff 7f 00 00 01` |
| 44 | 2 | `addr_recv.port` | **network byte order (BE)** `8333` = `20 8d` |
| 46 | 8 | `addr_trans.services` uint64 LE | `0` |
| 54 | 16 | `addr_trans.ip` | same as recv |
| 70 | 2 | `addr_trans.port` | **BE** `20 8d` |
| 72 | 8 | `nonce` uint64 LE | `0` |
| 80 | 1+ | `user_agent` | Bitcoin `var_str` = **CompactSize** (not Core VARINT) + bytes. We send CompactSize `17` + `/Bitcoin-Toolkit:4.0.0/` |
| 104 | 4 | `start_height` int32 LE | `0` |
| 108 | 1 | `relay` bool | `0` |

Payload length = 109. Inbound parse uses the same layout (variable UA length).

**Frozen unit-test vector** (timestamp `1700000000`, nonce `0` — tests **must not** call `time(NULL)`):

Full message including 24-byte header (magic `f9beb4d9`, command `version` + 5 NULs, length `6d000000`, checksum `d2a9d2ea`):

```
f9beb4d976657273696f6e00000000006d000000d2a9d2ea7f110100000000000000000000f1536500000000000000000000000000000000000000000000ffff7f000001208d000000000000000000000000000000000000ffff7f000001208d0000000000000000172f426974636f696e2d546f6f6c6b69743a342e302e302f0000000000
```

Payload only (109 bytes):

```
7f110100000000000000000000f1536500000000000000000000000000000000000000000000ffff7f000001208d000000000000000000000000000000000000ffff7f000001208d0000000000000000172f426974636f696e2d546f6f6c6b69743a342e302e302f0000000000
```

P2P framing (mainnet only): magic on the wire `f9 be b4 d9`; 12-byte command; uint32 LE length; checksum = first 4 of HASH256(payload).

**Service bits:** 0 `NODE_NETWORK`, 1 `NODE_GETUTXO`, 2 `NODE_BLOOM`, 3 `NODE_WITNESS`, 4 `NODE_XTHIN`, 6 `NODE_COMPACT_FILTERS`, 10 `NODE_NETWORK_LIMITED`. Every other set bit is listed as `BIT_<n>` (decimal n). Named and `BIT_<n>` entries appear in ascending bit order. Do not omit unknown bits.

### 5. `btk help` — Phase 5

```text
btk help
btk help <command>
```

Plain text on stdout. Not JSON. Exact text is *Appendix C*. Unknown topic: exit 1, `btk help: unknown command 'foo'`.

`btk <command> --help` prints the same body as `btk help <command>` from Phase 1 onward (each command carries its section). Phase 5 only adds the `help` command and the overview.

Newly written man pages wrap the same text. `make install` installs them. Help never execs `man`.

### 6. `btk version` — Phase 6

```text
btk version
btk --version
```

Emits the `version` object. `--out plain` prints `4.0.0`. `--version` works before Phase 6 as a stub that prints `4.0.0\n` so the binary is identifiable; Phase 6 replaces the stub with the typed command.

### 7. `btk balance` — Phases 7a–7d

Local address → satoshi index.

```text
btk balance [--path DIR] [item…]                    # query
btk balance --build --from-rpc [--host H] [--port P] [--rpc-auth USER:PASS]
btk balance --build --from-chainstate [--chainstate DIR]
btk balance --update                                 # RPC, from last height+1
```

| Long | Default |
|---|---|
| `--path` | config `balance.path` or `~/.btk/balance` |
| `--host` / `--port` | config `rpc.host` / `rpc.port` or `127.0.0.1` / `8332` |
| `--rpc-auth` | config `rpc.auth` (form `user:pass`; we Base64 at request time). No cookie file. |
| `--chainstate` | config `chainstate.path` or `~/.bitcoin/chainstate` |
| `--force` | Only with `--build`. Overwrite a non-empty `--path`. |

Query is a **transformer**: argv items or stdin. `type=address` → `data`; `type=balance` → `address`; bare string → parse as address. Empty stdin and no argv → empty stdout, exit 0. Query does **not** open a write handle. Missing address → `sats: 0`. Missing database → `balance database not found (run btk balance --build)`.

`--build` refuses to overwrite a non-empty directory unless `--force`.

**Illegal combinations**

| Combo | Error |
|---|---|
| `--from-rpc` and `--from-chainstate` | `specify one of --from-rpc or --from-chainstate` |
| `--build` without a source | `--build requires --from-rpc or --from-chainstate` |
| `--force` without `--build` | `--force requires --build` |
| `--update` with `--build` | `cannot combine --update and --build` |
| `--update` and no database | `balance database not found (run btk balance --build)` |
| `--from-rpc` / `--from-chainstate` without `--build` | `--from-rpc requires --build` (same for chainstate) |

Progress goes to **stderr** so pipes stay clean:

```text
building: height 800000/850000 (94.1%)
complete: height 850000
```

`\r` on the progress line is fine. Final `complete` line ends with `\n`.

#### Storage (new layout — not compatible with 3.1.2)

LevelDB, directory `--path`.

| Key | Value |
|---|---|
| `A` \|\| UTF-8 address | `uint64` satoshis, little-endian |
| `O` \|\| 32-byte txid (internal/LE) \|\| `uint32` vout LE | CompactSize(addr len) \|\| addr \|\| `uint64` amount LE |
| `Mheight` | `uint32` LE last consumed height |
| `Mtip` | 32-byte tip hash (internal) |

`A` is the query path. `O` is the UTXO/outpoint map so `--update` can debit the right address when an input spends. Both writers (RPC and chainstate) use this layout.

**txid is BIP-141:** HASH256 of the **non-witness** serialization (`nVersion || vin || vout || nLockTime`). Never hash marker/flag/witness. Display-hex txids in logs are byte-reversed; keys store internal order.

#### Who gets an `A` row

Extract a standard address from `scriptPubKey`:

| Script | Address |
|---|---|
| `OP_DUP OP_HASH160 20 <h> OP_EQUALVERIFY OP_CHECKSIG` | P2PKH |
| `OP_HASH160 20 <h> OP_EQUAL` | P2SH (index only) |
| `OP_0 20 <h>` | P2WPKH |
| `OP_0 32 <h>` | P2WSH (index only) |
| `OP_1 32 <x>` | P2TR: encode the 32-byte `x` **as-is** (bech32m v1). Do **not** apply a BIP-341 tweak — the program on chain is already the output key. |
| `<33/65-byte pubkey> OP_CHECKSIG` | P2PKH of that pubkey (historical P2PK) |

Anything else is skipped (not an error). Network for encoding follows the node we are indexing: **mainnet** in 4.0.0. (Testnet index is a later `--network` on `--build`.)

#### Build from chainstate (Bitcoin Core ≥ 0.15)

Require a **stopped** bitcoind or a copy. Read-only.

1. Open LevelDB at `--chainstate`.
2. Read obfuscation key: raw key `0x0e 0x00` + `obfuscate_key`. Value is `0x08` + 8 XOR bytes. XOR every subsequent key and value, cycling those 8 bytes.
3. Best block: deobfuscated key `'B'` → 32-byte tip. Record as `Mtip`. Height: we do not have a height index in chainstate; after iteration take the **max** decoded UTXO height and write `Mheight`. Also print it.
4. UTXO records: deobfuscated key `'C'` (0x43) + 32-byte txid + Bitcoin Core **varint** vout. If a `'c'` (0x63) key appears, abort: `unsupported chainstate format (pre-0.15)`.
5. Value (Bitcoin Core `Coin` compression):
   - Core varint `nCode`. `height = nCode >> 1`, `coinbase = nCode & 1`.
   - Core varint compressed amount → decompress (algorithm below).
   - Compressed script (`nSize` + payload) → `scriptPubKey` → address.

**Amount decompress** (Bitcoin Core `DecompressAmount`):

```
if x == 0: return 0
x -= 1
e = x % 10
x /= 10
if e < 9:
    d = (x % 9) + 1
    x /= 9
    n = x * 10 + d
else:
    n = x + 1
while e:
    n *= 10
    e -= 1
return n
```

**Script decompress:**

| nSize | Payload | scriptPubKey |
|---|---|---|
| 0 | 20 bytes | P2PKH |
| 1 | 20 bytes | P2SH |
| 2 or 3 | 32 bytes x | compressed pubkey `02/03\|\|x` then wrap as P2PK (`<pk> OP_CHECKSIG`) — we then address it as P2PKH of that pk |
| 4 or 5 | 32 bytes x | Uncompressed P2PK. **Do not** implement √mod p. Build the 33-byte compressed key `02\|\|x` (nSize 4) or `03\|\|x` (nSize 5), then `secp256k1_ec_pubkey_parse` + `secp256k1_ec_pubkey_serialize(UNCOMPRESSED)` and wrap as `<65-byte pk> OP_CHECKSIG`. Parse failure → skip the UTXO. |
| ≥ 6 | nSize − 6 bytes | raw script |

6. Single-threaded iterate is fine for correctness. Batch-write `A` and `O`. Sum amounts per address (a second UTXO to the same address adds).

**Bitcoin CompactSize** (P2P / blocks / txs / `var_str` — **not** chainstate):

| n | Encoding |
|---|---|
| `< 253` | 1 byte `n` |
| `≤ 0xffff` | `0xfd` + uint16 LE |
| `≤ 0xffffffff` | `0xfe` + uint32 LE |
| else | `0xff` + uint64 LE |

Goldens: `0 → 00`, `23 → 17`, `252 → fc`, `253 → fd fd 00`.

**Bitcoin Core VARINT** (chainstate keys/values only — **not** CompactSize):

Read:

```
n = 0
loop:
    ch = next byte
    n = (n << 7) | (ch & 0x7f)
    if ch & 0x80: n = n + 1
    else: return n
```

Write:

```
// produce least-significant group first in tmp[0], then emit tmp[len] .. tmp[0]
len = 0
loop:
    tmp[len] = (n & 0x7f) | (len ? 0x80 : 0x00)
    if n <= 0x7f: break
    n = (n >> 7) - 1
    len++
emit tmp[len] .. tmp[0]
```

Goldens: `0 → 00`, `1 → 01`, `127 → 7f`, `128 → 80 00`, `255 → 80 7f`, `256 → 81 00`, `200 → 80 48`.

#### Block and tx layout (needed for RPC `--build` and BIP-141 txid)

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

**BIP-141 txid** = HASH256(`nVersion || vin || vout || nLockTime`) — strip marker, flag, and every witness. **wtxid** = HASH256(the full serialization). Appendix A.10 is the frozen pair.

#### Build / update from RPC

HTTP/1.0 or 1.1, `Content-Type: application/json`, `Authorization: Basic <base64(user:pass)>`.

Methods:

- `getblockcount` → tip height
- `getblockhash height` → hash
- `getblock hash 0` → raw block hex (we parse; one code path, our BIP-141 txid)

`--build --from-rpc` walks `0 … tip`. `--update` walks `Mheight+1 … tip`. If the RPC tip hash at `Mheight` does not match `Mtip`, abort: `reorg detected; rebuild with --build --force`.

For each block, for each tx, for each input (skip coinbase): look up `O[prevout]`; if found, debit that address by that amount (floor at 0 and warn on stderr if missing — a missing prevout on a full `--build` from genesis should not happen; on `--update` it means the DB was incomplete). Delete the outpoint. For each output with a recognized address: credit `A[addr]`, write `O[this_outpoint]`.

**Concurrency:** one fetch thread, one parse thread, one DB-writer thread. Shared queue of parsed block effects, mutex + condvar, max depth 100. No lock-free shared lists.

LevelDB missing at compile time: the command prints `btk balance: this build was compiled without LevelDB (install libleveldb-dev and rebuild)` and exits 1.

### 8. `btk config` — Phase 8

```text
btk config set <key>=<value>
btk config unset <key>
btk config get <key>
btk config dump
```

Allowed keys:

| Key | Meaning |
|---|---|
| `rpc.host` | JSON-RPC host |
| `rpc.port` | JSON-RPC port (integer) |
| `rpc.auth` | `user:pass` |
| `balance.path` | Index directory |
| `chainstate.path` | Bitcoin Core chainstate directory |

File: `$BTK_CONFIG` or `--config` or `~/.btk/config.json`. Nested JSON. Types: `rpc.host` string, `rpc.port` JSON number (integer 1–65535), `rpc.auth` string, `balance.path` string, `chainstate.path` string. Unknown keys on disk are ignored. Unknown keys on `set`/`unset`/`get`: `unknown config key 'foo'`.

Create the file (mode `0600`) and parents (`0700`) only on `set`. Missing file:

- `dump` → a `config` object with no data keys (only `"type":"config"`). No mkdir. Exit 0.
- `get <key>` → `no such key` (even if the key name is valid). No mkdir. Exit 1.
- `unset` → `no such key`. No mkdir. Exit 1.

`get <key>` when the file exists: `--out plain` of the raw stored value (so `get rpc.port` prints `8332`). `get rpc.auth` prints `********`. Missing key in an existing file: `no such key`.

`dump` always emits the typed `config` object (dotted keys). `--out plain` is one `key=value` per line. `rpc.auth` is **always** eight asterisks when present; omitted when unset. No `--show-secrets`. No cookie file in 4.0.0.

On-disk example (`~/.btk/config.json`):

```json
{
  "rpc": {
    "host": "127.0.0.1",
    "port": 8332,
    "auth": "alice:s3cret"
  },
  "balance": { "path": "/home/alice/.btk/balance" },
  "chainstate": { "path": "/home/alice/.bitcoin/chainstate" }
}
```

Corresponding `btk config dump` (ndjson):

```json
{"type":"config","rpc.host":"127.0.0.1","rpc.port":8332,"rpc.auth":"********","balance.path":"/home/alice/.btk/balance","chainstate.path":"/home/alice/.bitcoin/chainstate"}
```

Corresponding `--out plain`:

```
rpc.host=127.0.0.1
rpc.port=8332
rpc.auth=********
balance.path=/home/alice/.btk/balance
chainstate.path=/home/alice/.bitcoin/chainstate
```

---

## Crypto

```mermaid
flowchart LR
    PK[privkey / pubkey / p2tr] --> SECP[libsecp256k1]
    HASH[HASH160 / HASH256 / tagged_hash] --> SHA[SHA-256 in-tree]
    HASH --> RMD[RIPEMD-160 in-tree]
    RAND[privkey --new] --> GE[getentropy /dev/urandom]
```

**libsecp256k1**

- `secp256k1_context_create(SECP256K1_CONTEXT_NONE)` (or `SIGN\|VERIFY` on older headers).
- `secp256k1_ec_seckey_verify`, `secp256k1_ec_pubkey_create`, `secp256k1_ec_pubkey_parse` / `serialize`.
- Taproot: `secp256k1_xonly_pubkey_parse`, `secp256k1_xonly_pubkey_tweak_add`. Link `-lsecp256k1`. Extra extrakeys module is in the default distro package.

**Curve order**

```
n = FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141
```

**Hashes to implement** (test against Appendix A):

- `SHA256(msg)` — FIPS 180-4
- `RIPEMD160(msg)`
- `HASH256(msg) = SHA256(SHA256(msg))`
- `HASH160(msg) = RIPEMD160(SHA256(msg))`
- `tagged_hash(tag, msg)` as BIP-340

No GMP. 32-byte scalars are `std::array<uint8_t,32>`. Compare to `n` via `secp256k1_ec_seckey_verify` rather than a home-rolled bigint, except WIF/hex parse which only needs a 32-byte buffer.

---

## API / Interface Changes

There is no 3.1.2 compatibility surface. Migration for humans (README):

| 3.1.2 | 4.0.0 |
|---|---|
| `btk privkey --create` | `btk privkey --new` |
| `btk privkey --create --stream \| btk address --trace --grep='^1bri'` | `btk privkey --new --stream \| btk address --type p2pkh --match '^1bri'` |
| `btk address --bech32` | `btk address --type p2wpkh` (default) |
| `btk address --legacy` | `btk address --type p2pkh` |
| `btk address --bech32m` | **removed** (was not Taproot). Use `--type p2tr`. |
| `btk address --p2tr` (if anyone used a patch) | `btk address --type p2tr` |
| `btk node --hostname=X` | `btk node X` |
| `btk privkey "passphrase"` | `btk privkey --from-text "passphrase"` |
| `cat photo.jpg \| btk privkey --in-type=binary` | `btk privkey --from-file photo.jpg` |
| `--out-format=list` | `--out plain` |
| `--testnet` | `--network testnet` |
| JSON array of strings | typed objects |

Worked pipes:

```bash
# key → pubkey → address (typed objects)
btk privkey --new | btk pubkey | btk address --type p2wpkh

# same, bare strings
btk privkey --new --out plain | btk address --type p2tr --out plain

# both compressions of one secret
btk privkey --new --compressed --uncompressed

# convert WIF → hex
btk privkey --encoding hex KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn

# vanity (prints the matching address + source privkey)
btk privkey --new --stream | btk address --type p2pkh --match '^1bri'

# taproot vanity
btk privkey --new --stream | btk address --type p2tr --match '^bc1p73'

# node
btk node seed.bitcoin.sipa.be

# balance
btk balance --build --from-chainstate
btk balance 1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa
```

---

## Data Model Changes

No 3.1.2 database is migrated.

- **Config:** `~/.btk/config.json` (new path vs `~/.btk/btk.conf`). Nested JSON. Mode 0600.
- **Balance:** `~/.btk/balance/` LevelDB as specified. Rebuild from chainstate or RPC. 3.1.2 TXOA files are trash after the wipe.
- **No in-tree fixture copied from `test/balance/`.** Phase 7 tests create a tiny DB with the new writer and query it.

---

## Alternatives Considered

### A. Keep 3.1.2 CLI, rewrite internals

**Pros:** existing scripts survive. **Cons:** locks in `--bech32m` lying about Taproot, untyped string arrays, short-opt soup, silent passphrase hashing. The previous spec took this path and was rejected. **Not chosen.**

### B. Subcommand groups (`btk key new`, `btk chain tip`)

**Pros:** git-like grouping, room to grow. **Cons:** Phase order is one *command* per phase (privkey, pubkey, address…). Extra nesting without extra jobs. **Not chosen for 4.0.0.** Revisit if HD/PSBT ever land.

### C. Default pretty JSON array + optional NDJSON

**Pros:** prettier screenshots. **Cons:** arrays are hostile to streaming; vanity then needs flush archaeology. **Not chosen.** Default NDJSON; `--out json` is the pretty form.

### D. Vendor nlohmann/json or write a parser from scratch

nlohmann is a 20k-line single header. A from-scratch parser is a bug magnet for a CLI that accepts user JSON. **picojson** is ~1k lines, header-only, BSD-2-Clause. **Chosen.**

### E. Optional OpenSSL for hashes

3.1.2 already paid for this (legacy provider, Makefile probes). Two hash functions are small and well-specified. **In-tree hashes chosen.**

### F. Keep terminal QR

Requires a vendor (Nayuki) for a feature `qrencode` already does. **Dropped.**

### G. Fresh cJSON vs picojson

The wipe forbids *copying* `src/mods/cJSON`; it does not ban cJSON as a project. A new download of Dave Gamble’s cJSON would work. picojson is one header, no `.c` to compile, BSD-2-Clause, and matches our tiny schema. **picojson chosen.**

### H. SQLite vs LevelDB for the new balance store

SQLite would give SQL and one file. LevelDB is what Bitcoin Core’s chainstate already is, so Phase 7c’s reader and 7b’s writer share a link and an iteration style. One optional package (`libleveldb-dev`) covers both. **LevelDB chosen.**

### I. RPC-only balance (skip the chainstate parser)

Avoids VARINT, obfuscation, and `Coin` compression. A mainnet `--build --from-rpc` walks ~850k hex blocks and takes days; the intended first build is “point at a stopped Core datadir.” **RPC is `--update` and small/test chains; chainstate is the initial path.**

### J. Core cookie file vs `user:pass`

Core’s default RPC auth is a cookie at `~/.bitcoin/.cookie`. Convenient, but another path + race with a running node. 4.0.0 stores `rpc.auth=user:pass` (first-colon split) and Base64s it at request time. **No cookie file.**

---

## Security & Privacy

| Threat | Severity | Mitigation |
|---|---|---|
| Invalid scalar (`0`, `≥ n`) accepted as a key | High | `secp256k1_ec_seckey_verify` on generate and parse. |
| Homemade EC bugs | High | Only libsecp256k1 multiplies points. |
| Secrets on stderr / in argv traces | High | Error prefix is command name only. Never echo WIF/hex/passphrases/`rpc.auth`. |
| Config file world-readable | High | `0600` on the file, `0700` on `~/.btk`. |
| `config dump` leaking RPC password | High | Always `********`. `get rpc.auth` redacts too. |
| `--from-text` used as a brainwallet | Medium | Allowed but explicit. README warns: SHA-256(passphrase) is not a KDF. |
| CSPRNG failure ignored | High | Short `getentropy`/`urandom` read is fatal. |
| Balance data race / torn writes | Medium | Single writer thread, LevelDB write batches. |
| wtxid used as outpoint | High | BIP-141 non-witness txid only. |
| Reorg silently corrupts the index | Medium | Tip-hash check on `--update`; refuse and demand `--build --force`. |
| SSRF via `btk node` / RPC host | Low | User-supplied host; no URL fetch. IPv4 only. |
| Regex ReDoS on `--match` | Low | POSIX ERE; document that users pass the pattern. |

Threat model: a local CLI run by the operator. Not a daemon. Not multi-tenant.

---

## Observability

Not a service. Observability is:

- **stderr** for errors and balance progress. Never mix with NDJSON stdout.
- No log file, no metrics daemon.
- `btk version` reports whether LevelDB was linked.

No alerts.

---

## Rollout Plan

### Wipe checklist (do this first)

1. Tag the current tree: `git tag legacy/3.1.2`.
2. Keep **exactly two files**: `LICENSE` (GPL-3) and this spec, written to `REBUILD.md` (**replace** the rejected 3.1.2-compat `REBUILD.md`; do not leave both). No sibling plans.
3. Delete everything else: `src/`, `test/`, `man/`, `Makefile`, `README.md`, `assets/`, `bin/`, `obj/`, `src/mods/cJSON`, `src/mods/QRCodeGen`, etc.
4. **Do not** copy `man/`, `test/`, `cJSON`, or `QRCodeGen` out of the old tree “to restore later.”
5. Fetch picojson at the pinned commit into `third_party/picojson/`.
6. Land the new `Makefile` + `README.md` + `src/` from Phase 1.

### Feature flags

None. Commands appear when their PR merges. A missing command is `unknown command`.

### Staged rollout

Ship 4.0.0 when Phases 1–8 are in. No 3.1.2 dual-stack.

### Rollback

`git checkout legacy/3.1.2`. The products do not share a database.

---

## PR Plan

Each phase after the scaffold is one command and the tests that make it real.

| PR | Phase | Delivers | Acceptance |
|---|---|---|---|
| **0** | Scaffold | Layout, Makefile, `main`/dispatcher, options, NDJSON I/O, hash, hex, base58, error, secp RAII, `third_party/picojson`, `--help`/`--version` stubs. `bin/btk` runs and rejects unknown commands. | `make` produces `bin/btk`. Unit tests for SHA256, RIPEMD160, HASH160, HASH256, Base58Check, hex. |
| **1** | Private keys | `cmd/privkey`, WIF, CSPRNG, `--new/--encoding/--network/--compressed/--from-text/--from-file/--stream/--count`. CLI tests. New `btk-privkey.1`. | Appendix A Vector G + Wiki + WIF-wiki + `test01`/`Secret Passphrase` round-trips. Range reject 0 and `n`. Stream flush test. |
| **2** | Public keys | `cmd/pubkey`. | Vector G and Wiki compressed/uncompressed hex. WIF → pubkey. Recompress. Testnet privkey object → pubkey object with `network=testnet`. |
| **3** | Addresses | `cmd/address`, bech32, bech32m, BIP-341 tweak, `--type`, `--match`. | BIP-173 P2WPKH of G, BIP-341 empty-tree (A.6), Wiki P2PKH, G P2TR (A.2), **odd-Y secret 6 P2TR (A.2b)**. Uncompressed+p2wpkh errors. 64-hex is a secret (negative test). Vanity pipe test with a fixture key whose P2PKH is known, not a live grind. |
| **4** | Node | `net/p2p`, `cmd/node`. Offline unit test of version message ser/de. Live test behind `BTK_RUN_NET=1`. | Parse/serialize the frozen 109-byte payload and full header+payload hex in the node section. Port is BE `208d`. UA is CompactSize. `make test` does not touch the network. |
| **5** | Help | `cmd/help`, overview text, man pages for remaining stubs. | `btk help` and `btk help privkey` match Appendix C. Works without `man` in `PATH`. |
| **6** | Version | `cmd/version` typed object. | `btk version --out plain` → `4.0.0`. |
| **7a** | Balance primitives | CompactSize, Core VARINT, block/tx (de)ser, BIP-141 txid. Unit tests only. | Appendix A.9–A.11 goldens. A.10: both HASH256 values, and a parse of the 192-byte hex yields 2 witness items (72 + 33). txid ≠ wtxid. |
| **7b** | Balance query | New LevelDB layout + `btk balance` query (`address` / `balance` objects and bare strings). | Write a tiny DB in the test, query known address, unknown → 0, `address \| balance` pipe. |
| **7c** | Chainstate `--build` | Obfuscation, `'B'`/`'C'`, amount + script decompress, nSize 4/5 via libsecp256k1. | Appendix A.12 record → `1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH` = 5000000000. Pre-0.15 `'c'` aborts. |
| **7d** | RPC `--build` / `--update` | Hex `getblock`, three-thread queue, reorg check, stderr progress. | Offline: parse A.10 as a 1-tx “block” body; do not hit the network. |
| **8** | Config | `cmd/config`, load only from `config` and `balance`. | `dump` redacts `rpc.auth`. `get` of missing file is `no such key`. Failed parse does not mkdir. |

PR 1 is implementable the day after the wipe from this file: all encodings, vectors, help body for privkey, Makefile flags, and the pipe schema are inlined.

Suggested implementation order *inside* PR 1: hash → hex → base58 → random → secp wrapper → privkey object → dispatcher I/O → `privkey` command → unit tests → CLI tests.

---

## Testing strategy (from scratch)

Two layers, both required from Phase 1. **New files.** Do not copy `test/Tests/*.py`.

### C++ unit tests (`test/unit/`)

No gtest. Each file is a `main` that returns 0/1. A tiny `CHECK(cond)` macro prints file:line and bails. `make test-unit` runs them.

Cover: hex, Base58Check, bech32/bech32m (BIP-173 / BIP-350 strings; uppercase decode-only vs lowercase encode), HASH160/HASH256/tagged_hash, WIF round-trip, pubkey of G and of secret 6, P2PKH/P2WPKH/P2TR of A.2 / A.2b / A.6, amount decompress (A.9), CompactSize + VARINT (A.11), BIP-141 txid/wtxid of A.10.

### Python CLI tests (`test/cli/`)

`test/runner.py` spawns `bin/btk`, captures stdout/stderr/exit. One module per command, grown per PR.

Assertions compare parsed JSON objects (not 3.1.2 string tables). `--out plain` tests compare lines.

Required cases per phase are listed in the PR table. Cross-cutting:

- Empty stdin → empty stdout, 0.
- `--match` that matches nothing → empty stdout, 0.
- Producer writes one NDJSON object, sleeps 2 s, exits; consumer must emit before 2 s (use a pipeline with `timeout`).
- No test opens a network socket unless `BTK_RUN_NET=1`.
- No test requires `man`.

### Balance fixtures

Create at test time with the 7b writer (one address, 50 BTC, one outpoint). Phase 7c feeds Appendix A.12 as a mock LevelDB (or a byte-level unit test of the decoder — a full LevelDB is not required if the decoder is tested on the plaintext+XOR bytes). Do not check in 3.1.2 `000005.ldb`.

---

## Open Questions

None remaining. Resolved:

1. **`btk balance --build --from-rpc` does not take `--network testnet` in 4.0.0.** RPC and address HRP stay mainnet. A later release can add it cheaply.
2. **`btk address` default `--type` stays `p2wpkh`.** Not `p2tr`.

---

## References

- BIP-13 / Base58Check addresses; Bitcoin Wiki *Wallet import format*; Bitcoin Wiki *Technical background of version 1 Bitcoin addresses*
- [BIP-141](https://github.com/bitcoin/bips/blob/master/bip-0141.mediawiki) SegWit txid vs wtxid
- [BIP-173](https://github.com/bitcoin/bips/blob/master/bip-0173.mediawiki) Bech32
- [BIP-340](https://github.com/bitcoin/bips/blob/master/bip-0340.mediawiki) `lift_x`, tagged hashes
- [BIP-341](https://github.com/bitcoin/bips/blob/master/bip-0341.mediawiki) Taproot tweak; [wallet-test-vectors.json](https://github.com/bitcoin/bips/blob/master/bip-0341/wallet-test-vectors.json)
- [BIP-350](https://github.com/bitcoin/bips/blob/master/bip-0350.mediawiki) Bech32m
- Bitcoin Core `src/compressor.cpp` (amount + script compression), `src/coins.h` (chainstate `Coin`), LevelDB XOR obfuscation (`0e00 obfuscate_key`)
- picojson `111c9be5188f7350c2eac9ddaedd8cca3d7bf394`
- libsecp256k1
- FIPS 180-4 (SHA-256); RIPEMD-160 (DBNS-94)

---

## Appendix A — Golden vectors

All values below were re-derived from the cited rules / BIPs. They are the acceptance oracles. Hex is lowercase unless a cited document used uppercase; implementations accept both and emit lowercase.

### A.1 Hash primitives

| Input | Algorithm | Digest |
|---|---|---|
| `abc` | SHA-256 | `ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad` |
| `abc` | RIPEMD-160 | `8eb208f7e05d987a9b044a8e98c6b087f15a0bfc` |
| `abc` | HASH256 | `4f8b42c22dd3729b519ba6f68d2da7cc5b2d606d05daed5ad5128cc03e6c6358` |
| `abc` | HASH160 | `bb1be98c142444d7a56aa3981c3942a978e4dc33` |
| empty | HASH256 | `5df6e0e2761359d30a8275058e299fcc0381534545f55cf43e41983f5d4c9456` |
| tag=`TapTweak`, msg=`79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798` | tagged_hash | `3cf5216d476a5e637bf0da674e50ddf55c403270dd36494dfcca438132fa30e7` |

HASH160 of G compressed (`0279be66…f81798`) is `751e76e8199196d454941c45d1b3a323f1433bd6` (BIP-173).

### A.2 Vector G — secp256k1 generator (secret `1`)

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
| P2WPKH main (BIP-173) | `bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4` |
| P2WPKH test (BIP-173) | `tb1qw508d6qejxtdg4y5r3zarvary0c5xw7kxpjzsx` |
| P2TR output x (tweaked) | `da4710964f7852695de2da025290e24af6d8c281de5a0b902b7135fd9fd74d21` |
| P2TR main | `bc1pmfr3p9j00pfxjh0zmgp99y8zftmd3s5pmedqhyptwy6lm87hf5sspknck9` |
| P2TR test | `tb1pmfr3p9j00pfxjh0zmgp99y8zftmd3s5pmedqhyptwy6lm87hf5ssk79hv2` |

Note: BIP-350’s `bc1p0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7vqzk5jj0` is the **untweaked** `x(G)` encoded as witness v1. That is a bech32m self-test, **not** a BIP-341 spendable P2TR for secret `1`. `btk address --type p2tr` must emit the tweaked address above.

Note: a commonly pasted testnet uncompressed WIF `91avARGdfge8E4tZfYLoxeJ5sGBdNJQH4kvjJoQFacbgx3cTMqe` decodes to secret **3**, not 1. Do not use it.

### A.3 Vector Wiki — Bitcoin Wiki version-1 address example

Secret `18e14a7b6a307f426a94f8114701e7c8e774e7f9a47e2c2035db29a206321725`

| Field | Value |
|---|---|
| WIF compressed main | `Kx45GeUBSMPReYQwgXiKhG9FzNXrnCeutJp4yjTd5kKxCitadm3C` |
| WIF uncompressed main | `5J1F7GHadZG3sCCKHCwg8Jvys9xUbFsjLnGec4H125Ny1V9nR6V` |
| WIF compressed test | `cNR4jZU2sR5goytD4wXT4aeKcbqGSekbxLxY69v8aryxTU1SMnJZ` |
| WIF uncompressed test | `91msh178DnLBqFhbuYqazuUwWpKBkRQvgj8bggdWMp81nVp9PfM` |
| pubkey compressed | `0250863ad64a87ae8a2fe83c1af1a8403cb53f53e486d8511dad8a04887e5b2352` |
| pubkey uncompressed | `0450863ad64a87ae8a2fe83c1af1a8403cb53f53e486d8511dad8a04887e5b23522cd470243453a299fa9e77237716103abc11a1df38855ed6f2ee187e9c582ba6` |
| P2PKH uncompressed (wiki) | `16UwLL9Risc3QfPqBUvKofHmBQ7wMtjvM` |
| P2PKH compressed | `1PMycacnJaSqwwJqjawXBErnLsZ7RkXUAs` |
| P2WPKH main | `bc1q7499s50fxu4c0qg23esvm5h8elvqkm33r2tdza` |
| P2TR output x | `a137269b60cc269ca2aaf9257d5554f3e660f9a80bf82ba95c05451465c52137` |
| P2TR main | `bc1p5ymjdxmqesnfeg42lyjh642570nxp7dgp0uzh22uq4z3gew9yymst6pshk` |
| P2TR test | `tb1p5ymjdxmqesnfeg42lyjh642570nxp7dgp0uzh22uq4z3gew9yymsujhlde` |

### A.4 Vector WIF-wiki

Secret `0c28fca386c7a227600b2fe50b7cae11ec86d3bf1fbe471be89827e19d72aa1d`

| Field | Value |
|---|---|
| WIF uncompressed main (wiki) | `5HueCGU8rMjxEXxiPuD5BDku4MkFqeZyd4dZ1jvhTVqvbTLvyTJ` |
| WIF compressed main | `KwdMAjGmerYanjeui5SHS7JkmpZvVipYvB2LJGU1ZxJwYvP98617` |
| pubkey compressed | `02d0de0aaeaefad02b8bdc8a01a1b8b11c696bd3d66a2c5f10780d95b7df42645c` |

### A.5 Vector `--from-text`

| Text (UTF-8) | SHA-256 (secret hex) | WIF compressed main |
|---|---|---|
| `test01` | `678e82d907d3e6e71f81d5cf3ddacc3671dc618c38a1b7a9f9393a83d025b296` | `Kzh1d5pXSZLtwsgENakrfCjuGy9txPEb3aEb2y8yyZo65qDs8bTu` |
| `Secret Passphrase` | `76ce9bba9487266738e3c4f0b3cfa4be0c0eba52ed1c3c425e06900442efe5e1` | `L1Cf21MBhiZX9QFTAhN3PGJkyvQzN4CuHwhasHsdV9tkEfiiB8Ug` |

Also: `test01` uncompressed main `5JbtoEnCt6yAWCUvKwKYeCitigTV5qzTHtwHKa7Lhuk4sYDnTpP`; compressed test `cR415zpNsd3A7K9VkzZz2XExuCTJcqLH7cP49PbVUgT6LaJiWb1Q`; uncompressed test `92NXNybkUL3JUFzCxHDTWoGrNLpCF1XedqoEQCTr3eV7ebgGHp5`.

### A.6 BIP-341 empty script tree (official wallet vector 0)

Use this as the P2TR unit-test primary.

| Field | Value |
|---|---|
| internal privkey | `6b973d88838f27366ed61c9ad6367663045cb456e28335c109e30717ae0c6baa` |
| internal x-only pubkey | `d6889cb081036e0faefa3a35157ad71086b123b2b144b649798b494c300a961d` |
| tweak | `b86e7be8f39bab32a6f2c0443abbc210f0edac0e2c53d501b36b64437d9c6c70` |
| tweaked output x | `53a1f6e454df1aa2776a2814a721372d6258050de330b3c6d10ee8f4e0dda343` |
| scriptPubKey | `512053a1f6e454df1aa2776a2814a721372d6258050de330b3c6d10ee8f4e0dda343` |
| address | `bc1p2wsldez5mud2yam29q22wgfh9439spgduvct83k3pm50fcxa5dps59h4z5` |

`btk privkey --encoding wif` of that hex, piped to `btk address --type p2tr`, must equal that address.

### A.2b Vector 6 — odd-Y compressed (`03`) P2TR

Secret `6` (`00…06`). Compressed prefix is `03` (odd Y). `lift_x` still uses even Y; a wrong `ec_pubkey_tweak_add` on the odd point will fail this row and pass A.2 / A.6.

| Field | Value |
|---|---|
| secret hex | `0000000000000000000000000000000000000000000000000000000000000006` |
| WIF compressed main | `KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU76Myig6zj` |
| WIF uncompressed main | `5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreBKdE2NK` |
| WIF compressed test | `cMahea7zqjxrtgAbB7LSGbcQUr1uX1ojuat9jZodMN87M73ZA41f` |
| WIF uncompressed test | `91avARGdfge8E4tZfYLoxeJ5sGBdNJQH4kvjJoQFacbgxPr9P7A` |
| pubkey compressed | `03fff97bd5755eeea420453a14355235d382f6472f8568a18b2f057a1460297556` |
| pubkey uncompressed | `04fff97bd5755eeea420453a14355235d382f6472f8568a18b2f057a1460297556ae12777aacfbb620f3be96017f45c560de80f0f6518fe4a03c870c36b075f297` |
| P2PKH compressed main | `1Cf2hs39Woi61YNkYGUAcohL2K2q4pawBq` |
| P2WPKH main | `bc1q0ldfeupqc9k2eaffep7cm6yml3ct3jwtwzqt7k` |
| tap tweak | `25fee0b2ff9076ea93b70323592f582d29a4139ce98af1843c67265ce1ba5843` |
| P2TR output x | `a8e1f6946495d797bda3c3c6a88cf34375130c57a42a966c9a0508bf3cc2fc1a` |
| P2TR main | `bc1p4rsld9ryjhte00drc0r23r8ngd63xrzh5s4fvmy6q5yt70xzlsdqcuvtzv` |
| P2TR test | `tb1p4rsld9ryjhte00drc0r23r8ngd63xrzh5s4fvmy6q5yt70xzlsdq056ycr` |

### A.7 Bech32 / Bech32m self-checks

**Decode-only** (uppercase HRP is valid input; a correct **encoder** must emit lowercase): `A12UEL5L` (bech32), `A1LQFN3A` (bech32m).

**Encode goldens** (lowercase): `a12uel5l`, `a1lqfn3a`, `abcdef1qpzry9x8gf2tvdw0s3jn54khce6mua7lmqqqxw` (bech32), `abcdef1l7aum6echk45nj3s0wdvt2fg8x9yrzpqzd3ryx` (bech32m).

Segwit decode **and** encode (BIP-350; emit lowercase):

| Address | scriptPubKey |
|---|---|
| `BC1QW508D6QEJXTDG4Y5R3ZARVARY0C5XW7KV8F3T4` | `0014751e76e8199196d454941c45d1b3a323f1433bd6` |
| `bc1p0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7vqzk5jj0` | `512079be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798` |

Invalid: mixed case; bech32 checksum on a v1 address; bech32m checksum on a v0 address (table in BIP-350).

### A.8 Rejected scalars

| Input | Result |
|---|---|
| 32 zero bytes / hex `00…00` | `private key out of range` |
| hex `n` = `ffffffff…4141` | `private key out of range` |
| hex `n+1` | `private key out of range` |

### A.9 Amount decompress (chainstate)

These match the `DecompressAmount` listing in the balance section (and Core’s inverse of `CompressAmount`). Do not “make a wrong table pass.”

| Compressed x | Satoshis |
|---|---|
| 0 | 0 |
| 1 | 1 |
| 2 | 10 |
| 3 | 100 |
| 9 | 100000000 |
| 10 | 1000000000 |
| 11 | 2 |
| 20 | 2000000000 |
| 50 | 5000000000 |

`50` is the `e == 9` row (50 × 10⁸ sats). Round-trip: `CompressAmount` of each right-hand value equals x.

### A.10 BIP-141 txid (handmade 1-in-1-out P2WPKH)

Version 2. One input: 32-byte zero prevout, vout 0, empty scriptSig, `nSequence = 0xffffffff`. One output: 100000 sats, P2WPKH of G (`0014` \|\| `751e76e8199196d454941c45d1b3a323f1433bd6`). Witness: two items — 72-byte dummy (`30` + 70 zero bytes + `01`) and G’s compressed pubkey. `nLockTime = 0`.

Non-witness serialization (txid preimage):

```
020000000100000000000000000000000000000000000000000000000000000000000000000000000000ffffffff01a086010000000000160014751e76e8199196d454941c45d1b3a323f1433bd600000000
```

Full witness serialization (wtxid preimage, **192 bytes**). After the 22-byte output script comes the per-input witness (`02` = two items, `48` = 72-byte dummy, `21` = 33-byte pubkey). There is **no** extra count byte between `scriptPubKey` and the stack — vin count implies one witness stack.

```
0200000000010100000000000000000000000000000000000000000000000000000000000000000000000000ffffffff01a086010000000000160014751e76e8199196d454941c45d1b3a323f1433bd60248300000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001210279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f8179800000000
```

| | Internal (HASH256, as stored) | Display (byte-reversed) |
|---|---|---|
| **txid** (use this) | `3c58a2ad2dfc2f132e6dd137844f6e6bd749e33672f5e24d5109cea990c282a8` | `a882c290a9ce09514de2f57236e349d76b6e4f8437d16d2e132ffc2dada2583c` |
| **wtxid** (do not use) | `dd22da01b8929076ae782656fb7dfd1d3fe12a39c895b38aa533dbaa7d0806a1` | `a106087daadb33a58ab395c8392ae13f1dfd7dfb562678ae769092b801da22dd` |

Assert txid ≠ wtxid. The indexer keys `O` with the **internal** txid. Phase 7a must HASH256 both preimages **and** round-trip-parse the 192-byte hex: one input, one output, **two** witness items of length 72 and 33.

### A.11 CompactSize and Core VARINT

| Value | CompactSize | Core VARINT |
|---|---|---|
| 0 | `00` | `00` |
| 1 | `01` | `01` |
| 23 | `17` | `17` |
| 127 | `7f` | `7f` |
| 128 | `80` | `80 00` |
| 200 | `c8` | `80 48` |
| 252 | `fc` | `80 7c` |
| 253 | `fd fd 00` | `80 7d` |
| 255 | `fd ff 00` | `80 7f` |
| 256 | `fd 00 01` | `81 00` |

### A.12 Synthetic chainstate record (no XOR on the obfuscation row)

Obfuscation key (8 bytes): `0102030405060708`.

| LevelDB key (raw) | LevelDB value (raw) | Notes |
|---|---|---|
| `0e006f62667573636174655f6b6579` (`0e00` \|\| `obfuscate_key`) | `080102030405060708` | **Not** XOR’d |
| XOR(`42`) = `43` | XOR(`abab…ab` × 32) = `aaa9a8afaeadaca3` repeating | deobfuscated key `'B'`, tip 32 × `0xab` |
| XOR(`431111…111100`) = `42131215141716191013121514171619101312151417161910131215141716191002` | XOR(`80483200751e76e8199196d454941c45d1b3a323f1433bd6`) = `814a3104701871e0189395d051921b4dd0b1a027f4453cde` | deobfuscated key `'C'` \|\| 32 × `0x11` \|\| VARINT(0); value nCode=200 (height 100), amount x=50 (5000000000 sats), nSize=0 + HASH160(G compressed) |

Plaintext UTXO value: `80 48 32 00 751e76e8199196d454941c45d1b3a323f1433bd6`.

After decode: address `1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH`, 5000000000 sats, outpoint txid 32 × `0x11` vout 0, height 100.

XOR is cyclic over the 8-byte key. `'B'` XOR `01` is `0x43` — a decoder that skips deobfuscation will mistake the tip for a `'C'` key; tests should catch that.

---

## Appendix B — Phase 1 implementation notes

Enough to start coding the day after the wipe.

### `src/version.hpp`

```cpp
#pragma once
#define BTK_VERSION_MAJOR 4
#define BTK_VERSION_MINOR 0
#define BTK_VERSION_PATCH 0
#define BTK_VERSION_STRING "4.0.0"
```

### I/O loop

```
if cmd.is_generator(opts):          # --new, --from-text, --from-file, node, …
    if --from-file -: raw = read_all_stdin(); emit sha256(raw) as one key; return
    if --from-text: emit sha256(str) as one key; return   # do not read stdin
    if --new: emit count keys (infinite if --stream and no --count); return
    # node / version / help / config / balance --build|--update: run once
    return

items = argv_positionals or read_stdin(opts.in)
if items empty: return 0
for item in items:
    obj = parse_item(item)          # JSON object or wrap bare string
    outs = cmd.run(opts, obj)
    for o in outs:
        if cmd is address and opts.match and not regex_search(o.data): continue
        if opts.no_source: o.erase("source")
        write_out(opts, o)          # ndjson line + flush
```

`--in json` with a leading `[`: `picojson::parse` the whole array, then walk elements (output still incremental). `--in auto`/`ndjson` with a leading `{`: parse one object, run, parse the next (vanity).

### `parse_item` for `privkey`

1. If object with `type` other than `privkey` → error `expected a privkey`.
2. If object: read `data` + `encoding` (or guess from `data`).
3. If bare string: WIF if Base58Check decodes to 33 or 34 bytes with version `80`/`EF`; else if `[0-9a-fA-F]{64}` then hex; else error `not a WIF or hex private key`.

### Makefile probe sketch

```make
HAVE_SECP := $(shell echo 'int main(){return 0;}' | \
  $(CXX) -x c++ - -lsecp256k1 -o /dev/null 2>/dev/null && echo 1)
ifeq ($(HAVE_SECP),)
  $(error libsecp256k1 is required — sudo apt-get install libsecp256k1-dev)
endif
```

### First CLI tests (`test/cli/test_privkey.py`)

1. `btk privkey --new --out plain` → one line, valid WIF, compressed main (`K` or `L`).
2. That WIF back into `btk privkey --encoding hex --out plain` → 64 hex chars; then back to WIF equals original.
3. Vector G hex → each of the four WIFs via `--network` / `--compressed` / `--uncompressed`.
4. `--from-text test01 --out plain` → `Kzh1d5pXSZLtwsgENakrfCjuGy9txPEb3aEb2y8yyZo65qDs8bTu`. Must **not** read stdin: `printf '{"type":"privkey","data":"00"}\n' | btk privkey --from-text test01 --out plain` still yields that one WIF.
5. `--from-text 'Secret Passphrase'` → `L1Cf21MBhiZX9QFTAhN3PGJkyvQzN4CuHwhasHsdV9tkEfiiB8Ug`.
6. hex `00…00` → exit 1, stderr contains `out of range`.
7. `--new --count 3` → three NDJSON objects, distinct `data`.
8. `printf '%s\n' <G-wif> <wiki-wif> | btk privkey --encoding hex` → two objects.
9. Flush: subprocess `privkey --new --stream` piped to a Python consumer that exits on first object; must return before a 2 s timeout.

---

## Appendix C — Embedded help text

Print exactly this (or an isomorphic wrap to 80 columns). This is also the body of the new man pages. Keep this appendix in lockstep with the command tables; CLI tests pin the overview and `privkey` bodies byte-for-byte and only require the other bodies to contain the flags listed in those tables.

### `btk help` / `btk --help`

```
btk — Bitcoin Toolkit 4.0.0

Usage:
  btk [--config PATH] <command> [options] [item...]

Commands:
  privkey   Create or convert private keys
  pubkey    Derive or recompress public keys
  address   Derive P2PKH, P2WPKH, or BIP-341 P2TR addresses
  node      Handshake a Bitcoin P2P peer (IPv4 mainnet)
  help      Show this help or help for a command
  version   Show version
  balance   Build or query a local address-balance index
  config    Get and set defaults (RPC, paths)

Output is one JSON object per line (ndjson) unless --out json|plain.
Pipes compose:  btk privkey --new | btk address --type p2wpkh

See 'btk help <command>' or 'btk <command> --help'.
```

### `btk help privkey`

```
btk privkey — create or convert private keys

Usage:
  btk privkey --new [--count N] [--stream]
              [--encoding wif|hex] [--network mainnet|testnet]
              [--compressed | --uncompressed]
  btk privkey [--encoding wif|hex] [--network mainnet|testnet]
              [--compressed | --uncompressed]
              [--from-text STR | --from-file PATH]
              [item...]

  --new              CSPRNG key in [1, n-1]
  --encoding         Output wif (default) or hex
  --network          mainnet (default) or testnet; WIF version byte
  --compressed       Set the WIF/pubkey compression flag (default)
  --uncompressed     Clear the flag; both flags emit two objects
  --from-text STR    SHA-256(STR) as the secret (not a KDF)
  --from-file PATH   SHA-256(file bytes); PATH - reads stdin as bytes
  --count N          With --new, emit N keys
  --stream           With --new, emit until SIGINT
  --out ndjson|json|plain
  --in  auto|ndjson|json|plain

  --new, --from-text, and --from-file are generators (no stdin objects).
  They cannot be combined. --from-file - hashes stdin as raw bytes.

Items are WIF, 64-char hex, or typed privkey objects. Unknown strings
are errors (they are not hashed unless --from-text).
```

### `btk help pubkey`

```
btk pubkey — derive or recompress public keys

Usage:
  btk pubkey [--compressed | --uncompressed] [item...]

Items are a privkey (object, WIF, hex), or a hex public key.
Default compression follows the input; flags override. Both flags
emit two objects. Output is hex (33 or 65 bytes).
```

### `btk help address`

```
btk address — derive addresses

Usage:
  btk address [--type p2pkh|p2wpkh|p2tr]...
              [--match REGEX] [--ignore-case]
              [--network mainnet|testnet]
              [--no-source]
              [item...]

  --type       Repeatable. Default: p2wpkh
               p2pkh   Base58Check HASH160(pubkey)
               p2wpkh  Bech32 v0 HASH160(compressed pubkey)
               p2tr    Bech32m v1 BIP-341 key-path (empty tree)
  --match      POSIX ERE on the address; drop non-matches
  --source     Include a source object even for bare-string input
  --no-source  Do not echo the parent key on the object

p2wpkh and p2tr require a compressed key.
64-hex is a private key, never an x-only public key.
There is no --bech32m flag; use --type p2tr for Taproot.

Vanity:
  btk privkey --new --stream | btk address --type p2pkh --match '^1bri'
```

### `btk help node`

```
btk node — Bitcoin P2P version handshake (IPv4 mainnet)

Usage:
  btk node <host[:port]>
  btk node --host HOST [--port 8333]

Connects, sends version, prints the peer's version as a typed object,
and closes. 15s timeout. Default port 8333.
Give the host as a positional or --host, not both.
--verbose includes raw P2P fields. --out plain prints ip:port.
```

### `btk help version`

```
btk version — print 4.0.0

Usage:
  btk version
  btk --version
```

### `btk help balance`

```
btk balance — local address → satoshi index

Usage:
  btk balance [--path DIR] <address...>
  btk balance --build --from-rpc [--host H] [--port P] [--rpc-auth user:pass]
  btk balance --build --from-chainstate [--chainstate DIR]
  btk balance --update

Query prints {"type":"balance","address":"…","sats":N}. Missing = 0.
Accepts address objects from a pipe, or bare address strings.
--build writes a new LevelDB at --path (default ~/.btk/balance).
--force overwrites a non-empty --path (only with --build).
--from-rpc walks blocks via Bitcoin Core JSON-RPC (hex blocks).
--from-chainstate reads a Core ≥0.15 UTXO set (bitcoind stopped).
--update applies new RPC blocks from the stored tip.
Progress is on stderr. Requires LevelDB at build time.
No cookie file; use --rpc-auth user:pass or config rpc.auth.
```

### `btk help config`

```
btk config — defaults for RPC and paths

Usage:
  btk config set <key>=<value>
  btk config unset <key>
  btk config get <key>
  btk config dump

Keys: rpc.host, rpc.port, rpc.auth, balance.path, chainstate.path
File: ~/.btk/config.json (or --config / $BTK_CONFIG), mode 0600.
dump and get redact rpc.auth as ********.
```

### `btk help help`

```
btk help — show command help

Usage:
  btk help
  btk help <command>
```

---

## Appendix D — Risks

| Risk | Severity | Mitigation |
|---|---|---|
| picojson rejects NDJSON edge cases (NaN, deep nest) | Low | We emit a tiny schema; parser tests cover our objects. |
| In-tree SHA-256/RIPEMD-160 bug | High | Vectors in A.1 are mandatory unit tests; HASH160 of G is cross-checked against BIP-173. |
| Distro `libsecp256k1` built without extrakeys | Medium | Probe `secp256k1_xonly_pubkey_tweak_add` in the Makefile; error with a rebuild hint. Most packages enable it. |
| Chainstate format drift (Core 28+) | Medium | Abort on unknown key prefixes; document ≥0.15 `'C'` keys. |
| Full-mainnet RPC `--build` takes days | Low | Document chainstate as the intended initial path; RPC is for `--update` and small test chains. |
| `--stream` + `--match` burns CPU | Low | Expected for vanity. No rate limit. |
| Users treat `--from-text` as a wallet | Medium | Help + README warning. |

---

*End of 4.0.0 product spec.*
