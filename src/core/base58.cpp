#include "core/base58.hpp"

#include "core/hash.hpp"
#include "util/error.hpp"

#include <algorithm>

namespace {

const char kAlphabet[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

int alphabet_value(char c) {
    if (c >= '1' && c <= '9') {
        return c - '1';
    }
    if (c >= 'A' && c <= 'H') {
        return c - 'A' + 9;
    }
    if (c >= 'J' && c <= 'N') {
        return c - 'J' + 17;
    }
    if (c >= 'P' && c <= 'Z') {
        return c - 'P' + 22;
    }
    if (c >= 'a' && c <= 'k') {
        return c - 'a' + 33;
    }
    if (c >= 'm' && c <= 'z') {
        return c - 'm' + 44;
    }
    return -1;
}

}  // namespace

std::string base58_encode(const std::uint8_t* data, std::size_t len) {
    std::size_t zeros = 0;
    while (zeros < len && data[zeros] == 0) {
        ++zeros;
    }

    std::vector<std::uint8_t> b58((len - zeros) * 138 / 100 + 2);
    std::size_t length = 0;
    for (std::size_t i = zeros; i < len; ++i) {
        int carry = data[i];
        std::size_t j = 0;
        for (std::size_t k = b58.size(); k-- > 0 && (carry != 0 || j < length); ++j) {
            carry += 256 * static_cast<int>(b58[k]);
            b58[k] = static_cast<std::uint8_t>(carry % 58);
            carry /= 58;
        }
        length = j;
    }

    auto it = b58.begin() + (b58.size() - length);
    while (it != b58.end() && *it == 0) {
        ++it;
    }

    std::string out;
    out.assign(zeros, '1');
    for (; it != b58.end(); ++it) {
        out.push_back(kAlphabet[*it]);
    }
    return out;
}

std::string base58_encode(const std::vector<std::uint8_t>& data) {
    return base58_encode(data.data(), data.size());
}

std::vector<std::uint8_t> base58_decode(const std::string& encoded) {
    std::size_t zeros = 0;
    while (zeros < encoded.size() && encoded[zeros] == '1') {
        ++zeros;
    }

    std::vector<std::uint8_t> b256((encoded.size() - zeros) * 733 / 1000 + 2);
    std::size_t length = 0;
    for (std::size_t i = zeros; i < encoded.size(); ++i) {
        const int v = alphabet_value(encoded[i]);
        if (v < 0) {
            throw BtkError("", "invalid base58");
        }
        int carry = v;
        std::size_t j = 0;
        for (std::size_t k = b256.size(); k-- > 0 && (carry != 0 || j < length); ++j) {
            carry += 58 * static_cast<int>(b256[k]);
            b256[k] = static_cast<std::uint8_t>(carry % 256);
            carry /= 256;
        }
        length = j;
    }

    auto it = b256.begin() + (b256.size() - length);
    while (it != b256.end() && *it == 0) {
        ++it;
    }

    std::vector<std::uint8_t> out(zeros, 0);
    out.insert(out.end(), it, b256.end());
    return out;
}

std::string base58check_encode(const std::vector<std::uint8_t>& payload) {
    const Hash256 sum = hash256(payload);
    std::vector<std::uint8_t> ext = payload;
    ext.insert(ext.end(), sum.begin(), sum.begin() + 4);
    return base58_encode(ext);
}

std::vector<std::uint8_t> base58check_decode(const std::string& encoded) {
    const std::vector<std::uint8_t> raw = base58_decode(encoded);
    if (raw.size() < 4) {
        throw BtkError("", "invalid WIF checksum");
    }
    const std::vector<std::uint8_t> payload(raw.begin(), raw.end() - 4);
    const Hash256 sum = hash256(payload);
    if (!std::equal(sum.begin(), sum.begin() + 4, raw.end() - 4)) {
        throw BtkError("", "invalid WIF checksum");
    }
    return payload;
}
