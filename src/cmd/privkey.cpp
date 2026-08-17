#include "cmd/privkey.hpp"

#include "core/hash.hpp"
#include "core/json_io.hpp"
#include "core/privkey.hpp"
#include "util/error.hpp"

#include <iostream>
#include <vector>

namespace {

const char kHelp[] = R"help(btk privkey — create or convert private keys

Usage:
  btk privkey --new [--count N] [--stream]
              [--encoding wif|hex|dec] [--network mainnet|testnet]
              [--compressed | --uncompressed]
  btk privkey [--encoding wif|hex|dec] [--network mainnet|testnet]
              [--compressed | --uncompressed]
              [--from wif|hex|dec|text|file]

Create a private key from the CSPRNG, convert encodings, or derive a
key from explicit bytes. Input is stdin only (no positional keys).

Without --new, each stdin item is determined in this order: WIF
(base58check, version 0x80/0xEF), 64-character hex (case-insensitive;
64-digit all-numeric is hex, not decimal), a decimal digit string,
then SHA-256 of the line. Piped binary (NUL, other C0 controls, or
invalid UTF-8) is SHA-256 of the entire stdin — same as --from file.
A WIF-shaped string with a bad checksum is an error, not a hash.
Scalar 0 or >= n is "private key out of range".

--from overrides that determination on bare lines. Typed JSON objects
(type=privkey) always win. --from text SHA-256s each line even when
the text looks like a key (printf 1 | btk privkey --from text).
--from file hashes the whole stream as one key. SHA-256 of a
passphrase is not a KDF; do not use it as a wallet.

Options:
  -h, --help             Show this help and exit
      --new              CSPRNG key in [1, n-1]. Does not read stdin.
                         Cannot combine with --from.
      --encoding FMT     Output encoding: wif (default), hex, or dec.
                         Decimal data is a digit string with no
                         leading zeros.
  -n, --network NET      mainnet (default) or testnet. Sets the WIF
                         version byte. Re-encodes WIF to this
                         network.
      --compressed       Set the WIF/pubkey compression flag
                         (default)
      --uncompressed     Clear the flag. Both flags emit two objects
                         (compressed first).
      --from TYPE        Force stdin type: wif, hex, dec, text, or
                         file. Default: determined from the input.
  -c, --count N          With --new, emit N keys. N must be >= 1.
  -s, --stream           With --new, emit until SIGINT. Combined with
                         --count N, emit exactly N.
  -o, --out FORMAT       ndjson (default), json, or plain. plain
                         prints the key data.
      --in FORMAT        auto (default), ndjson, json, or plain.
                         --from text|wif|hex|dec with auto is coerced
                         to plain. --from file does not read objects.

Examples:
  btk privkey --new
  btk privkey --new --count 5 --encoding hex
  printf 1 | btk privkey --encoding dec --out plain
  printf 1 | btk privkey --from text --out plain
  cat photo.jpg | btk privkey --from file --out plain
)help";

std::string output_encoding(const Options& opts) {
    if (opts.encoding.empty()) {
        return "wif";
    }
    return opts.encoding;
}

JsonObject privkey_object(const Privkey& key, const std::string& encoding) {
    JsonObject o;
    set_string(o, "type", "privkey");
    set_string(o, "encoding", encoding);
    set_string(o, "network", network_name(key.network));
    set_bool(o, "compressed", key.compressed);
    if (encoding == "hex") {
        set_string(o, "data", encode_secret_hex(key.secret));
    } else if (encoding == "dec") {
        set_string(o, "data", encode_secret_dec(key.secret));
    } else {
        set_string(o, "data", encode_wif(key));
    }
    return o;
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

std::vector<JsonObject> emit_key(const Options& opts, Privkey key) {
    if (opts.network_set) {
        key.network = opts.network;
    }
    const std::string enc = output_encoding(opts);
    std::vector<JsonObject> out;
    for (bool compressed : compression_modes(opts, key.compressed)) {
        Privkey copy = key;
        copy.compressed = compressed;
        out.push_back(privkey_object(copy, enc));
    }
    return out;
}

std::vector<std::uint8_t> read_all_stdin() {
    std::vector<std::uint8_t> buf;
    char tmp[4096];
    while (std::cin.read(tmp, sizeof tmp) || std::cin.gcount() > 0) {
        buf.insert(buf.end(), tmp, tmp + static_cast<std::size_t>(std::cin.gcount()));
    }
    return buf;
}

bool valid_from(const std::string& from) {
    return from.empty() || from == "wif" || from == "hex" || from == "dec" || from == "text" ||
           from == "file";
}

Privkey key_from_bare_string(const std::string& text, const Options& opts) {
    if (opts.from == "wif") {
        return decode_wif(text);
    }
    if (opts.from == "hex") {
        Privkey key;
        key.secret = decode_secret_hex(text);
        key.network = Network::Main;
        key.compressed = true;
        return key;
    }
    if (opts.from == "dec") {
        Privkey key;
        key.secret = decode_secret_dec(text);
        key.network = Network::Main;
        key.compressed = true;
        return key;
    }
    if (opts.from == "text") {
        return privkey_from_digest(sha256(text), Network::Main, true);
    }
    if (looks_like_privkey_text(text)) {
        return parse_privkey_string(text);
    }
    return privkey_from_digest(sha256(text), Network::Main, true);
}

Privkey parse_object(const JsonObject& item) {
    auto type_it = item.find("type");
    if (type_it == item.end() || !type_it->second.is<std::string>() ||
        type_it->second.get<std::string>() != "privkey") {
        throw BtkError("privkey", "expected a privkey");
    }
    auto data_it = item.find("data");
    if (data_it == item.end() || !data_it->second.is<std::string>()) {
        throw BtkError("privkey", "expected a privkey");
    }
    const std::string data = data_it->second.get<std::string>();

    std::string encoding;
    auto enc_it = item.find("encoding");
    if (enc_it != item.end() && enc_it->second.is<std::string>()) {
        encoding = enc_it->second.get<std::string>();
    }

    Privkey key;
    bool from_scalar = false;
    if (encoding == "hex") {
        key.secret = decode_secret_hex(data);
        key.network = Network::Main;
        key.compressed = true;
        from_scalar = true;
    } else if (encoding == "dec") {
        key.secret = decode_secret_dec(data);
        key.network = Network::Main;
        key.compressed = true;
        from_scalar = true;
    } else if (encoding == "wif") {
        key = decode_wif(data);
    } else if (!encoding.empty()) {
        throw BtkError("privkey", "invalid encoding");
    } else {
        key = key_from_bare_string(data, Options{});
        from_scalar = !looks_like_privkey_text(data) || data.size() == 64 ||
                      looks_like_decimal(data);
    }

    if (from_scalar) {
        auto net_it = item.find("network");
        if (net_it != item.end() && net_it->second.is<std::string>() &&
            is_network_name(net_it->second.get<std::string>())) {
            key.network = parse_network(net_it->second.get<std::string>());
        }
        auto c_it = item.find("compressed");
        if (c_it != item.end() && c_it->second.is<bool>()) {
            key.compressed = c_it->second.get<bool>();
        }
    }
    return key;
}

class PrivkeyCommand : public Command {
public:
    const char* name() const override { return "privkey"; }
    const char* summary() const override { return "Create or convert private keys"; }
    const char* help() const override { return kHelp; }

    void register_options(OptionSpec& spec) const override {
        spec.add(0, "new", false);
        spec.add(0, "encoding", true);
        spec.add(0, "compressed", false);
        spec.add(0, "uncompressed", false);
        spec.add(0, "from", true);
    }

    bool is_generator(const Options& opts) const override {
        return opts.flag_new || opts.from == "file";
    }

    void init(Options& opts) override {
        if (!valid_from(opts.from)) {
            throw BtkError("privkey", "invalid --from");
        }
        if (opts.flag_new && !opts.from.empty()) {
            throw BtkError("privkey", "cannot combine --new and --from");
        }
        if (opts.count_set && !opts.flag_new) {
            throw BtkError("privkey", "--count requires --new");
        }
        if (opts.stream && !opts.flag_new) {
            throw BtkError("privkey", "--stream requires --new");
        }
        if (!opts.positionals.empty()) {
            throw BtkError("privkey", "provide input on stdin");
        }
        if (!opts.encoding.empty() && opts.encoding != "wif" && opts.encoding != "hex" &&
            opts.encoding != "dec") {
            throw BtkError("privkey", "invalid --encoding");
        }
        if (!opts.from.empty() && opts.from != "file" && opts.in == InFormat::Auto) {
            opts.in = InFormat::Plain;
        }
    }

    std::vector<JsonObject> run(const Options& opts,
                                const std::optional<JsonObject>& item) override {
        if (opts.flag_new) {
            return emit_key(opts, generate_privkey(opts.network, true));
        }
        if (opts.from == "file") {
            const std::vector<std::uint8_t> bytes = read_all_stdin();
            const Network net = opts.network_set ? opts.network : Network::Main;
            return emit_key(opts, privkey_from_digest(sha256(bytes), net, true));
        }
        if (!item) {
            throw BtkError("privkey", "expected a privkey");
        }
        if (is_bare(*item)) {
            return emit_key(opts, key_from_bare_string(bare_text(*item), opts));
        }
        return emit_key(opts, parse_object(*item));
    }
};

}  // namespace

std::unique_ptr<Command> make_privkey_command() {
    return std::make_unique<PrivkeyCommand>();
}
