#include "chain/script.hpp"

#include "core/base58.hpp"
#include "core/bech32.hpp"
#include "core/hash.hpp"
#include "util/error.hpp"

namespace {

constexpr std::uint8_t kOp0 = 0x00;
constexpr std::uint8_t kOpDup = 0x76;
constexpr std::uint8_t kOpEqual = 0x87;
constexpr std::uint8_t kOpEqualverify = 0x88;
constexpr std::uint8_t kOpHash160 = 0xa9;
constexpr std::uint8_t kOpChecksig = 0xac;
constexpr std::uint8_t kOp1 = 0x51;

constexpr std::uint8_t kP2pkhVer = 0x00;
constexpr std::uint8_t kP2shVer = 0x05;

std::string encode_base58_hash(std::uint8_t ver, const std::uint8_t* h20) {
    std::vector<std::uint8_t> payload;
    payload.reserve(21);
    payload.push_back(ver);
    payload.insert(payload.end(), h20, h20 + 20);
    return base58check_encode(payload);
}

}  // namespace

std::optional<std::string> address_from_script(const std::vector<std::uint8_t>& script) {
    const std::size_t n = script.size();
    const std::uint8_t* s = script.data();

    // P2PKH: OP_DUP OP_HASH160 20 <h> OP_EQUALVERIFY OP_CHECKSIG
    if (n == 25 && s[0] == kOpDup && s[1] == kOpHash160 && s[2] == 20 && s[23] == kOpEqualverify &&
        s[24] == kOpChecksig) {
        return encode_base58_hash(kP2pkhVer, s + 3);
    }

    // P2SH: OP_HASH160 20 <h> OP_EQUAL
    if (n == 23 && s[0] == kOpHash160 && s[1] == 20 && s[22] == kOpEqual) {
        return encode_base58_hash(kP2shVer, s + 2);
    }

    // P2WPKH: OP_0 20 <h>
    if (n == 22 && s[0] == kOp0 && s[1] == 20) {
        return segwit_encode("bc", 0, std::vector<std::uint8_t>(s + 2, s + 22));
    }

    // P2WSH: OP_0 32 <h>
    if (n == 34 && s[0] == kOp0 && s[1] == 32) {
        return segwit_encode("bc", 0, std::vector<std::uint8_t>(s + 2, s + 34));
    }

    // P2TR: OP_1 32 <x> — encode x as-is, no tweak
    if (n == 34 && s[0] == kOp1 && s[1] == 32) {
        return segwit_encode("bc", 1, std::vector<std::uint8_t>(s + 2, s + 34));
    }

    // P2PK compressed: 33-byte pubkey OP_CHECKSIG
    if (n == 35 && s[0] == 33 && s[34] == kOpChecksig && (s[1] == 0x02 || s[1] == 0x03)) {
        const Hash160 h = hash160(s + 1, 33);
        return encode_base58_hash(kP2pkhVer, h.data());
    }

    // P2PK uncompressed: 65-byte pubkey OP_CHECKSIG
    if (n == 67 && s[0] == 65 && s[66] == kOpChecksig && s[1] == 0x04) {
        const Hash160 h = hash160(s + 1, 65);
        return encode_base58_hash(kP2pkhVer, h.data());
    }

    return std::nullopt;
}

std::optional<std::string> classify_mainnet_address(const std::string& s) {
    if (s.empty()) {
        return std::nullopt;
    }

    std::string hrp;
    int witver = 0;
    std::vector<std::uint8_t> program;
    if (segwit_decode(s, hrp, witver, program)) {
        if (hrp != "bc") {
            return std::nullopt;
        }
        if (witver == 0 && program.size() == 20) {
            return std::string("p2wpkh");
        }
        if (witver == 0 && program.size() == 32) {
            return std::string("p2wsh");
        }
        if (witver == 1 && program.size() == 32) {
            return std::string("p2tr");
        }
        return std::nullopt;
    }

    try {
        const std::vector<std::uint8_t> payload = base58check_decode(s);
        if (payload.size() != 21) {
            return std::nullopt;
        }
        if (payload[0] == kP2pkhVer) {
            return std::string("p2pkh");
        }
        if (payload[0] == kP2shVer) {
            return std::string("p2sh");
        }
        return std::nullopt;
    } catch (const BtkError&) {
        return std::nullopt;
    }
}

bool is_mainnet_address(const std::string& s) {
    if (s.empty()) {
        return false;
    }

    std::string hrp;
    int witver = 0;
    std::vector<std::uint8_t> program;
    if (segwit_decode(s, hrp, witver, program)) {
        return hrp == "bc";
    }

    try {
        const std::vector<std::uint8_t> payload = base58check_decode(s);
        if (payload.size() != 21) {
            return false;
        }
        return payload[0] == kP2pkhVer || payload[0] == kP2shVer;
    } catch (const BtkError&) {
        return false;
    }
}

std::optional<std::vector<std::uint8_t>> script_from_address(const std::string& addr) {
    if (addr.empty()) {
        return std::nullopt;
    }

    std::string hrp;
    int witver = 0;
    std::vector<std::uint8_t> program;
    if (segwit_decode(addr, hrp, witver, program)) {
        if (hrp != "bc") {
            return std::nullopt;
        }
        std::vector<std::uint8_t> script;
        if (witver == 0 && program.size() == 20) {
            script.push_back(kOp0);
            script.push_back(20);
            script.insert(script.end(), program.begin(), program.end());
            return script;
        }
        if (witver == 0 && program.size() == 32) {
            script.push_back(kOp0);
            script.push_back(32);
            script.insert(script.end(), program.begin(), program.end());
            return script;
        }
        if (witver == 1 && program.size() == 32) {
            script.push_back(kOp1);
            script.push_back(32);
            script.insert(script.end(), program.begin(), program.end());
            return script;
        }
        return std::nullopt;
    }

    try {
        const std::vector<std::uint8_t> payload = base58check_decode(addr);
        if (payload.size() != 21) {
            return std::nullopt;
        }
        std::vector<std::uint8_t> script;
        if (payload[0] == kP2pkhVer) {
            script.reserve(25);
            script.push_back(kOpDup);
            script.push_back(kOpHash160);
            script.push_back(20);
            script.insert(script.end(), payload.begin() + 1, payload.end());
            script.push_back(kOpEqualverify);
            script.push_back(kOpChecksig);
            return script;
        }
        if (payload[0] == kP2shVer) {
            script.reserve(23);
            script.push_back(kOpHash160);
            script.push_back(20);
            script.insert(script.end(), payload.begin() + 1, payload.end());
            script.push_back(kOpEqual);
            return script;
        }
        return std::nullopt;
    } catch (const BtkError&) {
        return std::nullopt;
    }
}

std::uint64_t dust_sats(const std::vector<std::uint8_t>& script_pubkey) {
    const std::size_t n = script_pubkey.size();
    const std::uint8_t* s = script_pubkey.data();
    if (n == 25 && s[0] == kOpDup && s[1] == kOpHash160 && s[2] == 20) {
        return 546;
    }
    if (n == 23 && s[0] == kOpHash160 && s[1] == 20) {
        return 540;
    }
    if (n == 22 && s[0] == kOp0 && s[1] == 20) {
        return 294;
    }
    if (n == 34 && s[0] == kOp0 && s[1] == 32) {
        return 330;
    }
    if (n == 34 && s[0] == kOp1 && s[1] == 32) {
        return 330;
    }
    return 546;
}
