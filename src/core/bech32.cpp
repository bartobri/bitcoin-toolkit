#include "core/bech32.hpp"

#include "util/error.hpp"

#include <cctype>

namespace {

constexpr const char kCharset[] = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

constexpr std::uint32_t kGen[5] = {0x3b6a57b2, 0x26508e6d, 0x1ea119fa, 0x3d4233dd, 0x2a1462b3};

constexpr std::uint32_t kBech32Const = 1;
constexpr std::uint32_t kBech32mConst = 0x2bc830a3;

std::uint32_t variant_const(Bech32Variant v) {
    return v == Bech32Variant::Bech32m ? kBech32mConst : kBech32Const;
}

std::uint32_t polymod(const std::vector<std::uint8_t>& values) {
    std::uint32_t chk = 1;
    for (std::uint8_t v : values) {
        const std::uint8_t b = static_cast<std::uint8_t>(chk >> 25);
        chk = ((chk & 0x1ffffffu) << 5) ^ v;
        for (int i = 0; i < 5; ++i) {
            if ((b >> i) & 1u) {
                chk ^= kGen[i];
            }
        }
    }
    return chk;
}

std::vector<std::uint8_t> hrp_expand(const std::string& hrp) {
    std::vector<std::uint8_t> out;
    out.reserve(hrp.size() * 2 + 1);
    for (unsigned char c : hrp) {
        out.push_back(static_cast<std::uint8_t>(c >> 5));
    }
    out.push_back(0);
    for (unsigned char c : hrp) {
        out.push_back(static_cast<std::uint8_t>(c & 31));
    }
    return out;
}

bool convertbits(const std::vector<std::uint8_t>& in, int from, int to, bool pad,
                 std::vector<std::uint8_t>& out) {
    std::uint32_t acc = 0;
    int bits = 0;
    const std::uint32_t maxv = (1u << to) - 1u;
    const std::uint32_t max_acc = (1u << (from + to - 1)) - 1u;
    out.clear();
    out.reserve((in.size() * static_cast<std::size_t>(from) + static_cast<std::size_t>(to) - 1) /
                static_cast<std::size_t>(to));
    for (std::uint8_t value : in) {
        if (value >> from) {
            return false;
        }
        acc = ((acc << from) | value) & max_acc;
        bits += from;
        while (bits >= to) {
            bits -= to;
            out.push_back(static_cast<std::uint8_t>((acc >> bits) & maxv));
        }
    }
    if (pad) {
        if (bits != 0) {
            out.push_back(static_cast<std::uint8_t>((acc << (to - bits)) & maxv));
        }
    } else if (bits >= from || ((acc << (to - bits)) & maxv) != 0) {
        return false;
    }
    return true;
}

int8_t charset_rev(unsigned char c) {
    if (c >= 'A' && c <= 'Z') {
        c = static_cast<unsigned char>(c - 'A' + 'a');
    }
    for (int i = 0; i < 32; ++i) {
        if (static_cast<unsigned char>(kCharset[i]) == c) {
            return static_cast<int8_t>(i);
        }
    }
    return -1;
}

}  // namespace

std::string bech32_encode(const std::string& hrp, const std::vector<std::uint8_t>& values,
                          Bech32Variant variant) {
    if (hrp.empty() || hrp.size() > 83) {
        throw BtkError("", "invalid bech32 hrp");
    }
    for (unsigned char c : hrp) {
        if (c < 33 || c > 126 || (c >= 'A' && c <= 'Z')) {
            throw BtkError("", "invalid bech32 hrp");
        }
    }
    for (std::uint8_t v : values) {
        if (v > 31) {
            throw BtkError("", "invalid bech32 data");
        }
    }

    std::vector<std::uint8_t> chk = hrp_expand(hrp);
    chk.insert(chk.end(), values.begin(), values.end());
    chk.insert(chk.end(), 6, 0);
    const std::uint32_t mod = polymod(chk) ^ variant_const(variant);

    std::string out = hrp;
    out.push_back('1');
    out.reserve(out.size() + values.size() + 6);
    for (std::uint8_t v : values) {
        out.push_back(kCharset[v]);
    }
    for (int i = 0; i < 6; ++i) {
        out.push_back(kCharset[(mod >> (5 * (5 - i))) & 31]);
    }
    if (out.size() > 90) {
        throw BtkError("", "invalid bech32 length");
    }
    return out;
}

bool bech32_decode(const std::string& encoded, std::string& hrp, std::vector<std::uint8_t>& values,
                   Bech32Variant& variant) {
    if (encoded.size() < 8 || encoded.size() > 90) {
        return false;
    }
    bool lower = false;
    bool upper = false;
    for (unsigned char c : encoded) {
        if (c < 33 || c > 126) {
            return false;
        }
        if (c >= 'a' && c <= 'z') {
            lower = true;
        } else if (c >= 'A' && c <= 'Z') {
            upper = true;
        }
    }
    if (lower && upper) {
        return false;
    }

    const auto sep = encoded.find_last_of('1');
    if (sep == std::string::npos || sep == 0 || sep + 7 > encoded.size()) {
        return false;
    }

    hrp.clear();
    hrp.reserve(sep);
    for (std::size_t i = 0; i < sep; ++i) {
        hrp.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(encoded[i]))));
    }

    std::vector<std::uint8_t> data;
    data.reserve(encoded.size() - sep - 1);
    for (std::size_t i = sep + 1; i < encoded.size(); ++i) {
        const int8_t v = charset_rev(static_cast<unsigned char>(encoded[i]));
        if (v < 0) {
            return false;
        }
        data.push_back(static_cast<std::uint8_t>(v));
    }

    std::vector<std::uint8_t> chk = hrp_expand(hrp);
    chk.insert(chk.end(), data.begin(), data.end());
    const std::uint32_t mod = polymod(chk);
    if (mod == kBech32Const) {
        variant = Bech32Variant::Bech32;
    } else if (mod == kBech32mConst) {
        variant = Bech32Variant::Bech32m;
    } else {
        return false;
    }
    values.assign(data.begin(), data.end() - 6);
    return true;
}

std::string segwit_encode(const std::string& hrp, int witver, const std::vector<std::uint8_t>& program) {
    if (witver < 0 || witver > 16) {
        throw BtkError("address", "invalid witness version");
    }
    if (program.size() < 2 || program.size() > 40) {
        throw BtkError("address", "invalid witness program");
    }
    if (witver == 0 && program.size() != 20 && program.size() != 32) {
        throw BtkError("address", "invalid witness program");
    }

    std::vector<std::uint8_t> data;
    data.push_back(static_cast<std::uint8_t>(witver));
    std::vector<std::uint8_t> converted;
    if (!convertbits(program, 8, 5, true, converted)) {
        throw BtkError("address", "invalid witness program");
    }
    data.insert(data.end(), converted.begin(), converted.end());
    const Bech32Variant variant = (witver == 0) ? Bech32Variant::Bech32 : Bech32Variant::Bech32m;
    return bech32_encode(hrp, data, variant);
}

bool segwit_decode(const std::string& addr, std::string& hrp, int& witver,
                   std::vector<std::uint8_t>& program) {
    std::vector<std::uint8_t> values;
    Bech32Variant variant;
    if (!bech32_decode(addr, hrp, values, variant) || values.empty()) {
        return false;
    }
    witver = values[0];
    if (witver > 16) {
        return false;
    }
    std::vector<std::uint8_t> prog5(values.begin() + 1, values.end());
    if (!convertbits(prog5, 5, 8, false, program)) {
        return false;
    }
    if (program.size() < 2 || program.size() > 40) {
        return false;
    }
    if (witver == 0) {
        if (variant != Bech32Variant::Bech32) {
            return false;
        }
        if (program.size() != 20 && program.size() != 32) {
            return false;
        }
    } else if (variant != Bech32Variant::Bech32m) {
        return false;
    }
    return true;
}
