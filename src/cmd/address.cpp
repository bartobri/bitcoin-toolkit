#include "cmd/address.hpp"

#include "core/address.hpp"
#include "core/json_io.hpp"
#include "core/privkey.hpp"
#include "core/pubkey.hpp"
#include "util/error.hpp"

#include <cctype>
#include <clocale>
#include <optional>
#include <regex.h>
#include <utility>
#include <vector>

namespace {

const char kHelp[] = R"help(btk address — derive addresses

Usage:
  btk address [--type p2pkh|p2wpkh|p2tr]...
              [--match REGEX] [--ignore-case]
              [--network mainnet|testnet]
              [--source]

Derive a Bitcoin address from an explicit private or public key.
Input is stdin only (no positional keys).

Items are a typed privkey or pubkey object (any encoding on the
object), or a bare line that is already a WIF private key or a
66- or 130-character hex public key. Determined in this order:
WIF, then hex pub.
There is no --from. 64-character hex, decimal, leftover text, and
binary stdin are errors (not keys). A WIF-shaped string with a bad
checksum is "invalid WIF checksum". 64-hex is never an x-only key.

--type is repeatable; default is one p2wpkh. Addresses are emitted
in flag order. p2wpkh and p2tr require a compressed key. p2tr is
BIP-341 key-path with an empty script tree (tweaked), encoded as
bech32m witness v1. There is no --bech32m flag.

Network: WIF version byte, else the typed object's network, else
--network, else mainnet. --network does not override a WIF version.
source is not walked.

--match is a POSIX extended regex on the address data, compiled in
the C locale. Non-matches are dropped (empty stdout is still exit
0). Implies --source. Pass --match at most once.

Options:
  -h, --help             Show this help and exit
      --type TYPE        Address script. Repeatable. Default: p2wpkh.
                         p2pkh   Base58Check HASH160(pubkey)
                         p2wpkh  Bech32 v0 HASH160(compressed pubkey)
                         p2tr    Bech32m v1 BIP-341 key-path
                                 (empty tree)
      --match REGEX      POSIX ERE on the address; drop non-matches;
                         includes source
      --ignore-case      Case-insensitive --match (REG_ICASE)
      --source           Include the input item as a source object
  -n, --network NET      mainnet (default) or testnet, when the input
                         does not already name a network
  -o, --out FORMAT       ndjson (default), json, or plain. plain
                         prints the address.
      --in FORMAT        auto (default), ndjson, json, or plain

Examples:
  btk privkey --new | btk address --type p2wpkh
  btk privkey --new | btk address --type p2pkh --type p2tr
  btk privkey --new --stream | btk address --type p2pkh --match '^1bri'
)help";

[[noreturn]] void fail_not_a_key() {
    throw BtkError("address", "not a private or public key");
}

template <typename F>
auto remap_key_error(F&& f) -> decltype(f()) {
    try {
        return f();
    } catch (const BtkError& err) {
        const std::string msg = err.what();
        if (msg == "private key out of range" || msg == "invalid WIF checksum") {
            throw BtkError("address", msg);
        }
        fail_not_a_key();
    }
}

std::optional<std::string> object_string(const JsonObject& item, const char* key) {
    auto it = item.find(key);
    if (it == item.end() || !it->second.is<std::string>()) {
        return std::nullopt;
    }
    return it->second.get<std::string>();
}

JsonObject priv_source(const std::string& encoding, const std::string& data, Network network,
                       bool compressed) {
    JsonObject o;
    set_string(o, "type", "privkey");
    set_string(o, "encoding", encoding);
    set_string(o, "network", network_name(network));
    set_bool(o, "compressed", compressed);
    set_string(o, "data", data);
    return o;
}

JsonObject pub_source(const std::string& data, Network network, bool compressed) {
    JsonObject o;
    set_string(o, "type", "pubkey");
    set_string(o, "encoding", "hex");
    set_string(o, "network", network_name(network));
    set_bool(o, "compressed", compressed);
    set_string(o, "data", data);
    return o;
}

struct InputKey {
    Pubkey pk;
    bool network_locked = false;
    std::optional<JsonObject> source;
};

Pubkey pubkey_from_priv(const Privkey& key) {
    return pubkey_from_secret(key.secret, key.compressed, key.network);
}

InputKey from_wif(const std::string& text) {
    const Privkey key = remap_key_error([&] { return decode_wif(text); });
    InputKey in;
    in.pk = pubkey_from_priv(key);
    in.network_locked = true;
    in.source = priv_source("wif", text, key.network, key.compressed);
    return in;
}

InputKey from_secret_hex(const std::string& text, Network network) {
    Privkey key;
    key.secret = remap_key_error([&] { return decode_secret_hex(text); });
    key.network = network;
    key.compressed = true;
    InputKey in;
    in.pk = pubkey_from_priv(key);
    in.source = priv_source("hex", text, key.network, key.compressed);
    return in;
}

InputKey from_secret_dec(const std::string& text, Network network) {
    Privkey key;
    key.secret = remap_key_error([&] { return decode_secret_dec(text); });
    key.network = network;
    key.compressed = true;
    InputKey in;
    in.pk = pubkey_from_priv(key);
    in.source = priv_source("dec", text, key.network, key.compressed);
    return in;
}

InputKey from_pub_hex(const std::string& text, Network network) {
    InputKey in;
    in.pk = remap_key_error([&] { return parse_pubkey_hex(text); });
    in.pk.network = network;
    in.source = pub_source(text, in.pk.network, in.pk.compressed);
    return in;
}

InputKey key_from_bare_string(const std::string& text, const Options& opts) {
    const Network fallback = opts.network_set ? opts.network : Network::Main;
    if (looks_like_wif(text)) {
        return from_wif(text);
    }
    if (looks_like_pubkey_hex(text)) {
        return from_pub_hex(text, fallback);
    }
    fail_not_a_key();
}

InputKey key_from_privkey_data(const std::string& text) {
    if (looks_like_wif(text)) {
        return from_wif(text);
    }
    if (text.size() == 64) {
        bool hex64 = true;
        for (unsigned char c : text) {
            if (!std::isxdigit(static_cast<unsigned char>(c))) {
                hex64 = false;
                break;
            }
        }
        if (hex64) {
            return from_secret_hex(text, Network::Main);
        }
    }
    if (looks_like_decimal(text)) {
        return from_secret_dec(text, Network::Main);
    }
    fail_not_a_key();
}

void apply_object_meta(InputKey& in, const JsonObject& item, bool allow_compressed_field) {
    auto net = object_string(item, "network");
    if (!in.network_locked && net && is_network_name(*net)) {
        in.pk.network = parse_network(*net);
        in.network_locked = true;
    }
    if (allow_compressed_field) {
        auto c_it = item.find("compressed");
        if (c_it != item.end() && c_it->second.is<bool>()) {
            in.pk = recompress_pubkey(in.pk, c_it->second.get<bool>());
        }
    }
}

InputKey parse_privkey_object(const JsonObject& item) {
    auto data = object_string(item, "data");
    if (!data) {
        fail_not_a_key();
    }
    const std::string encoding = object_string(item, "encoding").value_or("");

    InputKey in;
    if (encoding == "hex") {
        in = from_secret_hex(*data, Network::Main);
    } else if (encoding == "dec") {
        in = from_secret_dec(*data, Network::Main);
    } else if (encoding == "wif") {
        in = from_wif(*data);
    } else if (!encoding.empty()) {
        throw BtkError("address", "invalid encoding");
    } else {
        in = key_from_privkey_data(*data);
    }
    apply_object_meta(in, item, encoding != "wif");
    if (encoding == "wif") {
        auto c_it = item.find("compressed");
        if (c_it != item.end() && c_it->second.is<bool>()) {
            in.pk = recompress_pubkey(in.pk, c_it->second.get<bool>());
        }
    }
    in.source = item;
    return in;
}

InputKey parse_pubkey_object(const JsonObject& item) {
    auto data = object_string(item, "data");
    if (!data) {
        fail_not_a_key();
    }
    const std::string encoding = object_string(item, "encoding").value_or("");
    if (!encoding.empty() && encoding != "hex") {
        throw BtkError("address", "invalid encoding");
    }
    InputKey in = from_pub_hex(*data, Network::Main);
    apply_object_meta(in, item, true);
    in.source = item;
    return in;
}

InputKey parse_item(const JsonObject& item, const Options& opts) {
    if (is_bare(item)) {
        return key_from_bare_string(bare_text(item), opts);
    }
    const auto type = object_string(item, "type");
    if (type && *type == "privkey") {
        return parse_privkey_object(item);
    }
    if (type && *type == "pubkey") {
        return parse_pubkey_object(item);
    }
    fail_not_a_key();
}

JsonObject address_object(const std::string& data, AddressStyle style, Network network,
                          const std::optional<JsonObject>& source) {
    JsonObject o;
    set_string(o, "type", "address");
    set_string(o, "style", address_style_name(style));
    set_string(o, "network", network_name(network));
    set_string(o, "data", data);
    if (source) {
        o["source"] = JsonValue(*source);
    }
    return o;
}

bool match_keeps(const regex_t* re, bool compiled, const std::string& data) {
    if (!compiled) {
        return true;
    }
    return regexec(re, data.c_str(), 0, nullptr, 0) == 0;
}

std::vector<JsonObject> emit_address(const Options& opts, InputKey in, const regex_t* re,
                                     bool compiled) {
    if (!in.network_locked && opts.network_set) {
        in.pk.network = opts.network;
    }

    const std::optional<JsonObject> source =
        (opts.source || opts.match_set) ? in.source : std::nullopt;

    std::vector<JsonObject> out;
    for (const std::string& name : opts.types) {
        AddressStyle style;
        if (!parse_address_style(name, style)) {
            throw BtkError("address", "unknown address type");
        }
        const std::string data = encode_address(in.pk, style);
        if (!match_keeps(re, compiled, data)) {
            continue;
        }
        out.push_back(address_object(data, style, in.pk.network, source));
    }
    return out;
}

class AddressCommand : public Command {
public:
    ~AddressCommand() override {
        if (re_compiled_) {
            regfree(&re_);
        }
    }

    const char* name() const override { return "address"; }
    const char* summary() const override { return "Derive P2PKH, P2WPKH, or BIP-341 P2TR addresses"; }
    const char* help() const override { return kHelp; }

    void register_options(OptionSpec& spec) const override {
        spec.add(0, "type", true);
        spec.add(0, "match", true);
        spec.add(0, "ignore-case", false);
        spec.add(0, "source", false);
    }

    bool is_generator(const Options&) const override { return false; }

    void init(Options& opts) override {
        if (!opts.positionals.empty()) {
            throw BtkError("address", "provide input on stdin");
        }
        if (opts.count_set) {
            throw BtkError("address", "unknown option '--count'");
        }
        if (opts.types.empty()) {
            opts.types.emplace_back("p2wpkh");
        }
        for (const std::string& name : opts.types) {
            AddressStyle style;
            if (!parse_address_style(name, style)) {
                throw BtkError("address", "unknown address type");
            }
        }
        if (opts.match_set) {
            setlocale(LC_CTYPE, "C");
            setlocale(LC_COLLATE, "C");
            int flags = REG_EXTENDED | REG_NOSUB;
            if (opts.ignore_case) {
                flags |= REG_ICASE;
            }
            if (regcomp(&re_, opts.match.c_str(), flags) != 0) {
                throw BtkError("address", "invalid match pattern");
            }
            re_compiled_ = true;
        }
    }

    std::vector<JsonObject> run(const Options& opts,
                                const std::optional<JsonObject>& item) override {
        if (!item) {
            fail_not_a_key();
        }
        return emit_address(opts, parse_item(*item, opts), &re_, re_compiled_);
    }

private:
    regex_t re_{};
    bool re_compiled_ = false;
};

}  // namespace

std::unique_ptr<Command> make_address_command() {
    return std::make_unique<AddressCommand>();
}
