#include "cmd/pubkey.hpp"

#include "core/json_io.hpp"
#include "core/privkey.hpp"
#include "core/pubkey.hpp"
#include "util/error.hpp"

#include <cctype>
#include <utility>
#include <vector>

namespace {

const char kHelp[] = R"help(btk pubkey — derive or recompress public keys

Usage:
  btk pubkey [--compressed | --uncompressed]
             [--from wif|hex|dec] [--source]
             [--network mainnet|testnet]

Derive a secp256k1 public key from a private key, or parse and
recompress an existing public key. Input is stdin only (no positional
keys).

Items are a typed privkey or pubkey object, or a bare line that is
already a key. Determined in this order: WIF, 64-character hex
private key, decimal, then 66- or 130-character hex public key.
--from overrides that determination. --from hex accepts a 64-char
private key or a 66/130-char public key. 64-digit all-numeric is hex
priv. A 66- or 130-digit all-numeric string is decimal (determined
before hex pub).

This command does not invent a secret. Leftover text, --from text,
and --from file are errors. Hash a passphrase first with
btk privkey --from text, then pipe the typed object.

Default compression follows the input (WIF flag, object field, or
existing 02/03 vs 04 prefix). --compressed / --uncompressed override.
Both flags emit two objects (compressed first). Output encoding is
always hex (33 or 65 bytes).

Network comes from the typed object or WIF version byte, else
--network, else mainnet. --network does not override a WIF version.

Options:
  -h, --help             Show this help and exit
      --compressed       Emit a compressed public key (33 bytes)
      --uncompressed     Emit an uncompressed public key (65 bytes).
                         Both flags emit two objects.
      --from TYPE        Force bare-line type: wif, hex, or dec.
                         Default: determined from the input. hex is
                         64-char priv or 66/130-char pub.
      --source           Include the parent key as a source object
  -n, --network NET      mainnet (default) or testnet, when the input
                         does not already name a network
  -o, --out FORMAT       ndjson (default), json, or plain. plain
                         prints the hex public key.
      --in FORMAT        auto (default), ndjson, json, or plain.
                         --from with auto is coerced to plain.

Examples:
  btk privkey --new | btk pubkey
  btk privkey --new | btk pubkey --source
  printf '%s' <wif> | btk pubkey --out plain
)help";

[[noreturn]] void fail_not_a_key() {
    throw BtkError("pubkey", "not a private or public key");
}

template <typename F>
auto remap_privkey_error(F&& f) -> decltype(f()) {
    try {
        return f();
    } catch (const BtkError& err) {
        const std::string msg = err.what();
        if (msg == "private key out of range" || msg == "invalid WIF checksum") {
            throw BtkError("pubkey", msg);
        }
        fail_not_a_key();
    }
}

bool valid_from(const std::string& from) {
    return from.empty() || from == "wif" || from == "hex" || from == "dec";
}

std::vector<bool> compression_modes(const Options& opts, bool fallback) {
    if (opts.flag_compressed && opts.flag_uncompressed) {
        return {true, false};
    }
    if (opts.flag_uncompressed) {
        return {false};
    }
    if (opts.flag_compressed) {
        return {true};
    }
    return {fallback};
}

JsonObject pubkey_object(const Pubkey& pk, const std::optional<JsonObject>& source) {
    JsonObject o;
    set_string(o, "type", "pubkey");
    set_string(o, "encoding", "hex");
    set_string(o, "network", network_name(pk.network));
    set_bool(o, "compressed", pk.compressed);
    set_string(o, "data", encode_pubkey_hex(pk));
    if (source) {
        o["source"] = JsonValue(*source);
    }
    return o;
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

std::optional<std::string> object_string(const JsonObject& item, const char* key) {
    auto it = item.find(key);
    if (it == item.end() || !it->second.is<std::string>()) {
        return std::nullopt;
    }
    return it->second.get<std::string>();
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
    const Privkey key = remap_privkey_error([&] { return decode_wif(text); });
    InputKey in;
    in.pk = pubkey_from_priv(key);
    in.network_locked = true;
    in.source = priv_source("wif", text, key.network, key.compressed);
    return in;
}

InputKey from_secret_hex(const std::string& text, Network network) {
    Privkey key;
    key.secret = remap_privkey_error([&] { return decode_secret_hex(text); });
    key.network = network;
    key.compressed = true;
    InputKey in;
    in.pk = pubkey_from_priv(key);
    in.source = priv_source("hex", text, key.network, key.compressed);
    return in;
}

InputKey from_secret_dec(const std::string& text, Network network) {
    Privkey key;
    key.secret = remap_privkey_error([&] { return decode_secret_dec(text); });
    key.network = network;
    key.compressed = true;
    InputKey in;
    in.pk = pubkey_from_priv(key);
    in.source = priv_source("dec", text, key.network, key.compressed);
    return in;
}

InputKey from_pub_hex(const std::string& text, Network network) {
    InputKey in;
    in.pk = parse_pubkey_hex(text);
    in.pk.network = network;
    in.source = pub_source(text, in.pk.network, in.pk.compressed);
    return in;
}

InputKey key_from_bare_string(const std::string& text, const Options& opts) {
    const Network fallback = opts.network_set ? opts.network : Network::Main;
    if (opts.from == "wif") {
        return from_wif(text);
    }
    if (opts.from == "hex") {
        if (text.size() == 64) {
            return from_secret_hex(text, fallback);
        }
        if (looks_like_pubkey_hex(text)) {
            return from_pub_hex(text, fallback);
        }
        fail_not_a_key();
    }
    if (opts.from == "dec") {
        return from_secret_dec(text, fallback);
    }

    if (looks_like_wif(text)) {
        return from_wif(text);
    }
    if (text.size() == 64) {
        bool hex64 = true;
        for (unsigned char c : text) {
            if (!std::isxdigit(c)) {
                hex64 = false;
                break;
            }
        }
        if (hex64) {
            return from_secret_hex(text, fallback);
        }
    }
    if (looks_like_decimal(text)) {
        return from_secret_dec(text, fallback);
    }
    if (looks_like_pubkey_hex(text)) {
        return from_pub_hex(text, fallback);
    }
    fail_not_a_key();
}

void apply_object_meta(InputKey& in, const JsonObject& item, bool allow_compressed_field) {
    auto net = object_string(item, "network");
    if (net && is_network_name(*net)) {
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
        throw BtkError("pubkey", "invalid encoding");
    } else {
        in = key_from_bare_string(*data, Options{});
    }
    apply_object_meta(in, item, encoding != "wif");
    if (encoding == "wif") {
        auto c_it = item.find("compressed");
        if (c_it != item.end() && c_it->second.is<bool>()) {
            in.pk = recompress_pubkey(in.pk, c_it->second.get<bool>());
        }
    }
    JsonObject src = item;
    src.erase("source");
    in.source = std::move(src);
    return in;
}

InputKey parse_pubkey_object(const JsonObject& item) {
    auto data = object_string(item, "data");
    if (!data) {
        fail_not_a_key();
    }
    const std::string encoding = object_string(item, "encoding").value_or("");
    if (!encoding.empty() && encoding != "hex") {
        throw BtkError("pubkey", "invalid encoding");
    }
    InputKey in = from_pub_hex(*data, Network::Main);
    apply_object_meta(in, item, true);
    JsonObject src = item;
    src.erase("source");
    in.source = std::move(src);
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

std::vector<JsonObject> emit_pubkey(const Options& opts, InputKey in) {
    if (!in.network_locked && opts.network_set) {
        in.pk.network = opts.network;
    }
    const std::optional<JsonObject> source = opts.source ? in.source : std::nullopt;
    std::vector<JsonObject> out;
    for (bool compressed : compression_modes(opts, in.pk.compressed)) {
        Pubkey copy = recompress_pubkey(in.pk, compressed);
        copy.network = in.pk.network;
        out.push_back(pubkey_object(copy, source));
    }
    return out;
}

class PubkeyCommand : public Command {
public:
    const char* name() const override { return "pubkey"; }
    const char* summary() const override { return "Derive or recompress public keys"; }
    const char* help() const override { return kHelp; }

    void register_options(OptionSpec& spec) const override {
        spec.add(0, "compressed", false);
        spec.add(0, "uncompressed", false);
        spec.add(0, "from", true);
        spec.add(0, "source", false);
    }

    bool is_generator(const Options&) const override { return false; }

    void init(Options& opts) override {
        if (!valid_from(opts.from)) {
            throw BtkError("pubkey", "invalid --from");
        }
        if (!opts.positionals.empty()) {
            throw BtkError("pubkey", "provide input on stdin");
        }
        if (!opts.from.empty() && opts.in == InFormat::Auto) {
            opts.in = InFormat::Plain;
        }
    }

    std::vector<JsonObject> run(const Options& opts,
                                const std::optional<JsonObject>& item) override {
        if (!item) {
            fail_not_a_key();
        }
        return emit_pubkey(opts, parse_item(*item, opts));
    }
};

}  // namespace

std::unique_ptr<Command> make_pubkey_command() {
    return std::make_unique<PubkeyCommand>();
}
