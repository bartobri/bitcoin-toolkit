#include "core/privkey.hpp"

#include "core/base58.hpp"
#include "core/hash.hpp"
#include "core/hex.hpp"
#include "core/random.hpp"
#include "core/secp.hpp"
#include "util/error.hpp"

#include <algorithm>
#include <cctype>
#include <vector>

namespace {

constexpr std::uint8_t kWifMain = 0x80;
constexpr std::uint8_t kWifTest = 0xEF;

bool is_hex64(const std::string& s) {
    if (s.size() != 64) {
        return false;
    }
    for (unsigned char c : s) {
        if (!std::isxdigit(c)) {
            return false;
        }
    }
    return true;
}

bool mul10_add(Secret& acc, unsigned digit) {
    unsigned carry = digit;
    for (int i = 31; i >= 0; --i) {
        const unsigned v = static_cast<unsigned>(acc[static_cast<std::size_t>(i)]) * 10u + carry;
        acc[static_cast<std::size_t>(i)] = static_cast<std::uint8_t>(v);
        carry = v >> 8;
    }
    return carry == 0;
}

enum class WifResult { Ok, NotWif, BadChecksum, Invalid };

WifResult try_parse_wif(const std::string& s, Privkey& out) {
    std::vector<std::uint8_t> raw;
    try {
        raw = base58_decode(s);
    } catch (const BtkError&) {
        return WifResult::NotWif;
    }
    if (raw.size() < 4) {
        return WifResult::NotWif;
    }

    const std::vector<std::uint8_t> payload(raw.begin(), raw.end() - 4);
    const Hash256 sum = hash256(payload);
    const bool checksum_ok = std::equal(sum.begin(), sum.begin() + 4, raw.end() - 4);
    const bool wif_len = raw.size() == 37 || raw.size() == 38;

    if (!checksum_ok) {
        return wif_len ? WifResult::BadChecksum : WifResult::NotWif;
    }
    if (payload.size() != 33 && payload.size() != 34) {
        return WifResult::NotWif;
    }
    if (payload[0] != kWifMain && payload[0] != kWifTest) {
        return WifResult::NotWif;
    }
    if (payload.size() == 34 && payload[33] != 0x01) {
        return WifResult::Invalid;
    }

    std::copy(payload.begin() + 1, payload.begin() + 33, out.secret.begin());
    out.network = payload[0] == kWifTest ? Network::Test : Network::Main;
    out.compressed = payload.size() == 34;
    require_valid_secret(out.secret);
    return WifResult::Ok;
}

// True if this is WIF-shaped: 37/38-byte base58 payload, and either a bad
// checksum or a WIF version byte. Does not validate the scalar.
bool looks_like_wif_shape(const std::string& s) {
    std::vector<std::uint8_t> raw;
    try {
        raw = base58_decode(s);
    } catch (const BtkError&) {
        return false;
    }
    if (raw.size() != 37 && raw.size() != 38) {
        return false;
    }
    const std::vector<std::uint8_t> payload(raw.begin(), raw.end() - 4);
    const Hash256 sum = hash256(payload);
    const bool checksum_ok = std::equal(sum.begin(), sum.begin() + 4, raw.end() - 4);
    if (!checksum_ok) {
        return true;
    }
    return payload[0] == kWifMain || payload[0] == kWifTest;
}

}  // namespace

bool secret_is_valid(const Secret& secret) {
    return secp256k1_ec_seckey_verify(SecpContext::instance().get(), secret.data()) == 1;
}

void require_valid_secret(const Secret& secret) {
    if (!secret_is_valid(secret)) {
        throw BtkError("privkey", "private key out of range");
    }
}

std::string encode_wif(const Privkey& key) {
    std::vector<std::uint8_t> payload;
    payload.reserve(key.compressed ? 34 : 33);
    payload.push_back(key.network == Network::Test ? kWifTest : kWifMain);
    payload.insert(payload.end(), key.secret.begin(), key.secret.end());
    if (key.compressed) {
        payload.push_back(0x01);
    }
    return base58check_encode(payload);
}

Privkey decode_wif(const std::string& wif) {
    Privkey key;
    switch (try_parse_wif(wif, key)) {
    case WifResult::Ok:
        return key;
    case WifResult::BadChecksum:
        throw BtkError("privkey", "invalid WIF checksum");
    default:
        throw BtkError("privkey", "not a WIF, hex, or decimal private key");
    }
}

std::string encode_secret_hex(const Secret& secret) {
    return hex_encode(secret);
}

Secret decode_secret_hex(const std::string& hex) {
    if (hex.size() != 64) {
        throw BtkError("privkey", "invalid hex private key");
    }
    std::vector<std::uint8_t> raw;
    try {
        raw = hex_decode(hex);
    } catch (const BtkError&) {
        throw BtkError("privkey", "invalid hex private key");
    }
    Secret secret;
    std::copy(raw.begin(), raw.end(), secret.begin());
    require_valid_secret(secret);
    return secret;
}

bool looks_like_decimal(const std::string& text) {
    if (text.empty()) {
        return false;
    }
    for (unsigned char c : text) {
        if (c < '0' || c > '9') {
            return false;
        }
    }
    return true;
}

bool looks_like_privkey_text(const std::string& text) {
    return looks_like_wif_shape(text) || is_hex64(text) || looks_like_decimal(text);
}

std::string encode_secret_dec(const Secret& secret) {
    Secret acc = secret;
    std::string digits;
    bool nonzero = false;
    for (std::uint8_t b : acc) {
        if (b != 0) {
            nonzero = true;
            break;
        }
    }
    if (!nonzero) {
        return "0";
    }
    do {
        unsigned rem = 0;
        nonzero = false;
        for (std::size_t i = 0; i < acc.size(); ++i) {
            const unsigned v = (rem << 8) | acc[i];
            acc[i] = static_cast<std::uint8_t>(v / 10);
            rem = v % 10;
            if (acc[i] != 0) {
                nonzero = true;
            }
        }
        digits.push_back(static_cast<char>('0' + rem));
    } while (nonzero);
    std::reverse(digits.begin(), digits.end());
    return digits;
}

Secret decode_secret_dec(const std::string& dec) {
    if (!looks_like_decimal(dec)) {
        throw BtkError("privkey", "invalid decimal private key");
    }
    Secret acc{};
    bool started = false;
    for (char c : dec) {
        if (!started && c == '0') {
            continue;
        }
        started = true;
        if (!mul10_add(acc, static_cast<unsigned>(c - '0'))) {
            throw BtkError("privkey", "private key out of range");
        }
    }
    require_valid_secret(acc);
    return acc;
}

Privkey parse_privkey_string(const std::string& text) {
    Privkey key;
    switch (try_parse_wif(text, key)) {
    case WifResult::Ok:
        return key;
    case WifResult::BadChecksum:
        throw BtkError("privkey", "invalid WIF checksum");
    case WifResult::Invalid:
        throw BtkError("privkey", "not a WIF, hex, or decimal private key");
    case WifResult::NotWif:
        break;
    }
    if (is_hex64(text)) {
        key.secret = decode_secret_hex(text);
        key.network = Network::Main;
        key.compressed = true;
        return key;
    }
    if (looks_like_decimal(text)) {
        key.secret = decode_secret_dec(text);
        key.network = Network::Main;
        key.compressed = true;
        return key;
    }
    throw BtkError("privkey", "not a WIF, hex, or decimal private key");
}

Privkey generate_privkey(Network network, bool compressed) {
    Privkey key;
    key.network = network;
    key.compressed = compressed;
    do {
        random_bytes(key.secret.data(), key.secret.size());
    } while (!secret_is_valid(key.secret));
    return key;
}

Privkey privkey_from_digest(const Hash256& digest, Network network, bool compressed) {
    Privkey key;
    key.secret = digest;
    key.network = network;
    key.compressed = compressed;
    require_valid_secret(key.secret);
    return key;
}
