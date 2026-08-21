#include "cmd/sweep.hpp"

#include "chain/script.hpp"
#include "chain/sign.hpp"
#include "core/address.hpp"
#include "core/hex.hpp"
#include "core/json_io.hpp"
#include "core/privkey.hpp"
#include "core/pubkey.hpp"
#include "net/jsonrpc.hpp"
#include "util/config.hpp"
#include "util/error.hpp"

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <utility>

namespace {

const char kHelp[] = R"help(btk sweep — spend confirmed coins to one address

Usage:
  btk sweep --to ADDRESS
            [--fee-rate SATVB] [--dry-run]
            [--host H] [--port P] [--rpc-auth USER:PASS]
            [--type p2pkh|p2wpkh|p2tr]... [--source] [--verbose]

Sweep every confirmed UTXO at a spendable address to --to.
Looks up coins with Core scantxoutset (the node's UTXO set, not
~/.btk/balance). Signs in process. Broadcasts with
sendrawtransaction. Core does not need a wallet. Mainnet only.

Input is stdin only (no positional keys). Items are a typed address
object whose source contains a privkey, a typed privkey object, or
a bare WIF. Nested source is walked (up to 8 levels). The from
address is re-derived from the secret; a mismatch is "private key
does not match address". A typed privkey (or bare WIF) defaults to
one p2wpkh; --type is repeatable and selects which of that key's
addresses to sweep. p2sh and p2wsh cannot be spent.
64-hex, leftover text, a pubkey without a secret, and an address
without a privkey source are errors.

--to is required and may be p2pkh, p2sh, p2wpkh, p2wsh, or p2tr.
All mature UTXOs become inputs; one output is --to minus fee.
Fee is --fee-rate sat/vB, or estimatesmartfee 6. Dummy-signature
vsize is used so extra sats go to fee, not change. nLockTime is
the scan height; nSequence is 0xfffffffd (RBF). Immature coinbase
is skipped. Empty stdin is empty stdout, exit 0.

No cookie file. Use --rpc-auth user:pass or config rpc.auth.
--host / --port default to config rpc.host / rpc.port or
127.0.0.1 / 8332. A UTXO-set scan can take minutes.

Options:
  -h, --help             Show this help and exit
      --to ADDRESS       Destination mainnet address. Required.
      --fee-rate SATVB   Fee rate in sat/vB. Default: Core
                         estimatesmartfee 6
      --dry-run          Sign and print; do not broadcast
      --host HOST        RPC host. Default: config rpc.host or
                         127.0.0.1. A host:port form sets the port.
      --port PORT        RPC port. Default: config rpc.port or 8332
      --rpc-auth USER:PASS
                         HTTP Basic credentials. Default: config
                         rpc.auth. No cookie file.
      --type TYPE        With a privkey input, which address to
                         sweep. Repeatable. Default: p2wpkh
      --source           Include the input item as a source object
      --verbose          Include the raw transaction hex
      --config PATH      Config file. Default: $BTK_CONFIG, else
                         ~/.btk/config.json
  -o, --out FORMAT       ndjson (default), json, or plain. plain
                         prints the txid.
      --in FORMAT        auto (default), ndjson, json, or plain

Examples:
  btk privkey --new | btk address --source | btk sweep --to bc1q...
  printf '%s' KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn \
    | btk sweep --to 1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH --dry-run
)help";

[[noreturn]] void fail(const std::string& message) {
    throw BtkError("sweep", message);
}

[[noreturn]] void remap_key_error(const BtkError& err) {
    const std::string msg = err.what();
    if (msg == "private key out of range" || msg == "invalid WIF checksum") {
        fail(msg);
    }
    if (msg == "invalid hex private key" || msg == "invalid decimal private key" ||
        msg == "not a WIF, hex, or decimal private key") {
        fail("not a private key");
    }
    fail(msg);
}

std::optional<std::string> object_string(const JsonObject& item, const char* key) {
    auto it = item.find(key);
    if (it == item.end() || !it->second.is<std::string>()) {
        return std::nullopt;
    }
    return it->second.get<std::string>();
}

std::optional<bool> object_bool(const JsonObject& item, const char* key) {
    auto it = item.find(key);
    if (it == item.end() || !it->second.is<bool>()) {
        return std::nullopt;
    }
    return it->second.get<bool>();
}

const JsonObject* object_child(const JsonObject& item, const char* key) {
    auto it = item.find(key);
    if (it == item.end() || !it->second.is<JsonObject>()) {
        return nullptr;
    }
    return &it->second.get<JsonObject>();
}

const JsonObject* find_privkey_object(const JsonObject& item, int depth) {
    const auto type = object_string(item, "type");
    if (type && *type == "privkey") {
        return &item;
    }
    if (depth >= 8) {
        return nullptr;
    }
    const JsonObject* src = object_child(item, "source");
    if (src == nullptr) {
        return nullptr;
    }
    return find_privkey_object(*src, depth + 1);
}

Privkey privkey_from_object(const JsonObject& item) {
    auto data = object_string(item, "data");
    if (!data) {
        fail("not a private key");
    }
    const std::string encoding = object_string(item, "encoding").value_or("");
    Privkey key;
    try {
        if (encoding == "hex") {
            key.secret = decode_secret_hex(*data);
            key.compressed = true;
            key.network = Network::Main;
        } else if (encoding == "dec") {
            key.secret = decode_secret_dec(*data);
            key.compressed = true;
            key.network = Network::Main;
        } else if (encoding == "wif") {
            key = decode_wif(*data);
        } else if (!encoding.empty()) {
            fail("invalid encoding");
        } else {
            key = parse_privkey_string(*data);
        }
    } catch (const BtkError& err) {
        remap_key_error(err);
    }
    auto net = object_string(item, "network");
    if (encoding != "wif" && net && is_network_name(*net)) {
        key.network = parse_network(*net);
    }
    auto compressed = object_bool(item, "compressed");
    if (compressed) {
        key.compressed = *compressed;
    }
    return key;
}

Privkey privkey_from_wif(const std::string& text) {
    try {
        return decode_wif(text);
    } catch (const BtkError& err) {
        remap_key_error(err);
    }
}

AddressStyle require_spend_style(const std::string& name) {
    AddressStyle style;
    if (!parse_address_style(name, style)) {
        if (name == "p2sh" || name == "p2wsh") {
            fail(std::string("cannot spend ") + name);
        }
        fail("unknown address type");
    }
    return style;
}

std::string derive_address(const Privkey& key, AddressStyle style) {
    Pubkey pk;
    try {
        pk = pubkey_from_secret(key.secret, key.compressed, key.network);
        return encode_address(pk, style);
    } catch (const BtkError& err) {
        const std::string msg = err.what();
        if (msg == "uncompressed key cannot produce p2wpkh or p2tr" ||
            msg == "taproot tweak out of range") {
            fail(msg);
        }
        fail("not a private key");
    }
}

struct Spend {
    Privkey key;
    AddressStyle style = AddressStyle::P2wpkh;
    std::string from;
};

std::vector<Spend> spends_from_item(const JsonObject& item, const Options& opts) {
    if (is_bare(item)) {
        const std::string text = bare_text(item);
        if (looks_like_wif(text)) {
            Spend s;
            s.key = privkey_from_wif(text);
            std::vector<Spend> out;
            const std::vector<std::string>& types =
                opts.types.empty() ? std::vector<std::string>{"p2wpkh"} : opts.types;
            for (const std::string& name : types) {
                s.style = require_spend_style(name);
                s.from = derive_address(s.key, s.style);
                if (!is_mainnet_address(s.from)) {
                    fail("not a bitcoin address");
                }
                out.push_back(s);
            }
            return out;
        }
        if (is_mainnet_address(text) || classify_mainnet_address(text)) {
            fail("missing private key");
        }
        fail("not a private key");
    }

    const auto type = object_string(item, "type");
    if (!type) {
        fail("not a private key");
    }

    if (*type == "privkey") {
        Spend s;
        s.key = privkey_from_object(item);
        std::vector<Spend> out;
        const std::vector<std::string>& types =
            opts.types.empty() ? std::vector<std::string>{"p2wpkh"} : opts.types;
        for (const std::string& name : types) {
            s.style = require_spend_style(name);
            s.from = derive_address(s.key, s.style);
            if (!is_mainnet_address(s.from)) {
                fail("not a bitcoin address");
            }
            out.push_back(s);
        }
        return out;
    }

    if (*type == "address") {
        auto data = object_string(item, "data");
        if (!data || data->empty()) {
            fail("expected an address");
        }
        if (!is_mainnet_address(*data)) {
            fail("not a bitcoin address");
        }
        const auto style_name = object_string(item, "style").value_or("");
        const auto classified = classify_mainnet_address(*data);
        std::string name = style_name;
        if (name.empty() && classified) {
            name = *classified;
        }
        if (name == "p2sh" || name == "p2wsh") {
            fail(std::string("cannot spend ") + name);
        }
        AddressStyle style = require_spend_style(name.empty() ? "p2wpkh" : name);
        const JsonObject* pkobj = find_privkey_object(item, 0);
        if (pkobj == nullptr) {
            fail("missing private key");
        }
        Spend s;
        s.key = privkey_from_object(*pkobj);
        s.style = style;
        s.from = *data;
        const std::string derived = derive_address(s.key, s.style);
        if (derived != s.from) {
            fail("private key does not match address");
        }
        return {s};
    }

    if (*type == "pubkey") {
        const JsonObject* pkobj = find_privkey_object(item, 0);
        if (pkobj == nullptr) {
            fail("missing private key");
        }
        Spend s;
        s.key = privkey_from_object(*pkobj);
        std::vector<Spend> out;
        const std::vector<std::string>& types =
            opts.types.empty() ? std::vector<std::string>{"p2wpkh"} : opts.types;
        for (const std::string& name : types) {
            s.style = require_spend_style(name);
            s.from = derive_address(s.key, s.style);
            if (!is_mainnet_address(s.from)) {
                fail("not a bitcoin address");
            }
            out.push_back(s);
        }
        return out;
    }

    fail("not a private key");
}

Utxo utxo_from_rpc(const RpcUtxo& ru) {
    Utxo u;
    std::vector<std::uint8_t> raw;
    try {
        raw = hex_decode(ru.txid);
    } catch (const BtkError&) {
        fail("invalid rpc response");
    }
    if (raw.size() != 32) {
        fail("invalid rpc response");
    }
    for (std::size_t i = 0; i < 32; ++i) {
        u.txid_internal[i] = raw[31 - i];
    }
    u.vout = ru.vout;
    u.amount = ru.amount_sats;
    try {
        u.script_pubkey = hex_decode(ru.script_hex);
    } catch (const BtkError&) {
        fail("invalid rpc response");
    }
    u.coinbase = ru.coinbase;
    u.height = ru.height;
    return u;
}

bool coinbase_mature(const Utxo& u, std::uint32_t tip) {
    if (!u.coinbase) {
        return true;
    }
    if (tip < u.height) {
        return false;
    }
    return (tip - u.height + 1) >= 100;
}

JsonObject tx_object(const Transaction& tx, const std::string& from, const std::string& to,
                     std::uint64_t sats, std::uint64_t fee, bool broadcast,
                     const std::optional<JsonObject>& source, bool include_hex) {
    JsonObject o;
    set_string(o, "type", "tx");
    set_string(o, "txid", txid_display_hex(txid(tx)));
    set_string(o, "from", from);
    set_string(o, "to", to);
    set_uint64(o, "sats", sats);
    set_uint64(o, "fee", fee);
    set_uint64(o, "inputs", tx.vin.size());
    set_bool(o, "broadcast", broadcast);
    if (include_hex) {
        set_string(o, "hex", hex_encode(serialize_tx(tx, true)));
    }
    if (source) {
        o["source"] = JsonValue(*source);
    }
    return o;
}

class SweepCommand : public Command {
public:
    const char* name() const override { return "sweep"; }
    const char* summary() const override { return "Sweep confirmed coins to an address via Core RPC"; }
    const char* help() const override { return kHelp; }

    void register_options(OptionSpec& spec) const override {
        spec.add(0, "to", true);
        spec.add(0, "fee-rate", true);
        spec.add(0, "dry-run", false);
        spec.add(0, "host", true);
        spec.add(0, "port", true);
        spec.add(0, "rpc-auth", true);
        spec.add(0, "type", true);
        spec.add(0, "source", false);
        spec.add(0, "verbose", false);
    }

    bool is_generator(const Options&) const override { return false; }

    void init(Options& opts) override {
        if (!opts.positionals.empty()) {
            fail("provide input on stdin");
        }
        if (opts.count_set) {
            fail("unknown option '--count'");
        }
        if (opts.stream) {
            fail("sweep does not stream");
        }
        if (opts.to.empty()) {
            fail("missing destination");
        }
        dest_script_ = script_from_address(opts.to);
        if (!dest_script_) {
            fail("not a bitcoin address");
        }
        for (const std::string& name : opts.types) {
            require_spend_style(name);
        }

        const LoadedConfig cfg = load_config(opts);
        host_ = opts.host.empty() ? cfg.rpc_host : opts.host;
        port_ = opts.port_set ? opts.port : cfg.rpc_port;
        auth_ = opts.rpc_auth.empty() ? cfg.rpc_auth : opts.rpc_auth;
        const std::size_t colon = host_.find(':');
        if (colon != std::string::npos) {
            if (host_.find(':', colon + 1) != std::string::npos) {
                fail("invalid host");
            }
            if (opts.port_set) {
                fail("port specified twice");
            }
            const std::string suffix = host_.substr(colon + 1);
            host_ = host_.substr(0, colon);
            char* end = nullptr;
            errno = 0;
            const unsigned long v = std::strtoul(suffix.c_str(), &end, 10);
            if (end == suffix.c_str() || *end != '\0' || errno == ERANGE || v < 1 || v > 65535) {
                fail("invalid port");
            }
            port_ = static_cast<std::uint16_t>(v);
        }
        if (host_.empty()) {
            fail("invalid host");
        }
    }

    std::vector<JsonObject> run(const Options& opts,
                                const std::optional<JsonObject>& item) override {
        if (!item) {
            return {};
        }
        const std::vector<Spend> spends = spends_from_item(*item, opts);
        std::optional<JsonObject> source;
        if (opts.source) {
            if (is_bare(*item)) {
                JsonObject o;
                set_string(o, "type", "privkey");
                set_string(o, "encoding", "wif");
                set_string(o, "network", network_name(spends.front().key.network));
                set_bool(o, "compressed", spends.front().key.compressed);
                set_string(o, "data", bare_text(*item));
                source = std::move(o);
            } else {
                source = *item;
            }
        }
        std::vector<JsonObject> out;
        for (const Spend& sp : spends) {
            out.push_back(sweep_one(opts, sp, source));
        }
        return out;
    }

private:
    JsonObject sweep_one(const Options& opts, const Spend& sp,
                         const std::optional<JsonObject>& source) {
        const auto expect_script = script_from_address(sp.from);
        if (!expect_script) {
            fail("not a bitcoin address");
        }

        JsonRpc rpc(host_, port_, auth_, "sweep");
        rpc.set_timeout_ms(600000);

        std::cerr << "scanning utxo set\n";
        std::fflush(stderr);

        const ScanUtxoSet scan = rpc_scantxoutset(rpc, sp.from);
        if (!scan.success) {
            fail("utxo scan failed");
        }

        std::vector<Utxo> utxos;
        std::uint64_t total = 0;
        for (const RpcUtxo& ru : scan.unspents) {
            Utxo u = utxo_from_rpc(ru);
            if (u.script_pubkey != *expect_script) {
                fail("invalid rpc response");
            }
            if (!coinbase_mature(u, scan.height)) {
                continue;
            }
            if (u.amount > UINT64_MAX - total) {
                fail("insufficient funds");
            }
            total += u.amount;
            utxos.push_back(std::move(u));
        }
        if (utxos.empty()) {
            fail("no unspent outputs");
        }

        std::uint64_t satvb = opts.fee_rate;
        if (!opts.fee_rate_set) {
            const auto est = rpc_estimatesmartfee_satvb(rpc, 6);
            if (!est) {
                fail("fee rate unavailable (pass --fee-rate)");
            }
            satvb = *est;
        }

        const Pubkey pk = pubkey_from_secret(sp.key.secret, sp.key.compressed, sp.key.network);
        const std::uint64_t vsize = estimate_sweep_vsize(sp.style, pk, utxos.size(), *dest_script_);
        if (vsize > 100000) {
            fail("transaction too large");
        }
        if (satvb > 0 && vsize > UINT64_MAX / satvb) {
            fail("insufficient funds");
        }
        const std::uint64_t fee = vsize * satvb;
        if (fee >= total) {
            fail("insufficient funds");
        }
        const std::uint64_t sats = total - fee;
        if (sats < dust_sats(*dest_script_)) {
            fail("insufficient funds");
        }

        Transaction tx;
        try {
            tx = sign_sweep(sp.key, sp.style, utxos, *dest_script_, static_cast<std::int64_t>(sats),
                            scan.height);
        } catch (const BtkError& err) {
            fail(err.what());
        }

        const bool include_hex = opts.verbose || opts.dry_run;
        if (opts.dry_run) {
            return tx_object(tx, sp.from, opts.to, sats, fee, false, source, include_hex);
        }
        rpc_sendrawtransaction(rpc, hex_encode(serialize_tx(tx, true)));
        return tx_object(tx, sp.from, opts.to, sats, fee, true, source, include_hex);
    }

    std::string host_;
    std::uint16_t port_ = 8332;
    std::string auth_;
    std::optional<std::vector<std::uint8_t>> dest_script_;
};

}  // namespace

std::unique_ptr<Command> make_sweep_command() {
    return std::make_unique<SweepCommand>();
}
