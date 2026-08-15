#include "core/pubkey.hpp"

#include "core/hex.hpp"
#include "core/secp.hpp"
#include "util/error.hpp"

#include <cctype>

namespace {

bool is_hex_len(const std::string& s, std::size_t n) {
    if (s.size() != n) {
        return false;
    }
    for (unsigned char c : s) {
        if (!std::isxdigit(c)) {
            return false;
        }
    }
    return true;
}

Pubkey serialize_parsed(const secp256k1_pubkey& parsed, bool compressed, Network network) {
    Pubkey out;
    out.network = network;
    out.compressed = compressed;
    out.serialized.resize(compressed ? 33 : 65);
    std::size_t len = out.serialized.size();
    const unsigned int flags =
        compressed ? SECP256K1_EC_COMPRESSED : SECP256K1_EC_UNCOMPRESSED;
    if (secp256k1_ec_pubkey_serialize(SecpContext::instance().get(), out.serialized.data(), &len,
                                      &parsed, flags) != 1) {
        throw BtkError("pubkey", "not a private or public key");
    }
    out.serialized.resize(len);
    return out;
}

}  // namespace

bool looks_like_pubkey_hex(const std::string& text) {
    return is_hex_len(text, 66) || is_hex_len(text, 130);
}

Pubkey pubkey_from_secret(const Secret& secret, bool compressed, Network network) {
    if (!secret_is_valid(secret)) {
        throw BtkError("pubkey", "private key out of range");
    }
    secp256k1_pubkey parsed;
    if (secp256k1_ec_pubkey_create(SecpContext::instance().get(), &parsed, secret.data()) != 1) {
        throw BtkError("pubkey", "private key out of range");
    }
    return serialize_parsed(parsed, compressed, network);
}

Pubkey parse_pubkey_hex(const std::string& hex) {
    if (!looks_like_pubkey_hex(hex)) {
        throw BtkError("pubkey", "not a private or public key");
    }
    std::vector<std::uint8_t> raw;
    try {
        raw = hex_decode(hex);
    } catch (const BtkError&) {
        throw BtkError("pubkey", "not a private or public key");
    }
    secp256k1_pubkey parsed;
    if (secp256k1_ec_pubkey_parse(SecpContext::instance().get(), &parsed, raw.data(), raw.size()) !=
        1) {
        throw BtkError("pubkey", "not a private or public key");
    }
    const bool compressed = raw.size() == 33;
    return serialize_parsed(parsed, compressed, Network::Main);
}

Pubkey recompress_pubkey(const Pubkey& pk, bool compressed) {
    if (pk.compressed == compressed && !pk.serialized.empty()) {
        return pk;
    }
    secp256k1_pubkey parsed;
    if (secp256k1_ec_pubkey_parse(SecpContext::instance().get(), &parsed, pk.serialized.data(),
                                  pk.serialized.size()) != 1) {
        throw BtkError("pubkey", "not a private or public key");
    }
    return serialize_parsed(parsed, compressed, pk.network);
}

std::string encode_pubkey_hex(const Pubkey& pk) {
    return hex_encode(pk.serialized);
}
