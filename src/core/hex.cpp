#include "core/hex.hpp"

#include "util/error.hpp"

#include <cctype>

namespace {

int hex_nibble(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

}  // namespace

std::string hex_encode(const std::uint8_t* data, std::size_t len) {
    static const char kDigits[] = "0123456789abcdef";
    std::string out;
    out.resize(len * 2);
    for (std::size_t i = 0; i < len; ++i) {
        out[i * 2] = kDigits[data[i] >> 4];
        out[i * 2 + 1] = kDigits[data[i] & 0x0f];
    }
    return out;
}

std::string hex_encode(const std::vector<std::uint8_t>& data) {
    return hex_encode(data.data(), data.size());
}

std::vector<std::uint8_t> hex_decode(const std::string& hex) {
    if (hex.size() % 2 != 0) {
        throw BtkError("", "invalid hex");
    }
    std::vector<std::uint8_t> out(hex.size() / 2);
    for (std::size_t i = 0; i < out.size(); ++i) {
        const int hi = hex_nibble(hex[i * 2]);
        const int lo = hex_nibble(hex[i * 2 + 1]);
        if (hi < 0 || lo < 0) {
            throw BtkError("", "invalid hex");
        }
        out[i] = static_cast<std::uint8_t>((hi << 4) | lo);
    }
    return out;
}
