#include "core/address.hpp"

#include "core/base58.hpp"
#include "core/bech32.hpp"
#include "core/hash.hpp"
#include "core/secp.hpp"
#include "util/error.hpp"

#include <algorithm>
#include <secp256k1_extrakeys.h>

namespace {

constexpr std::uint8_t kP2pkhMain = 0x00;
constexpr std::uint8_t kP2pkhTest = 0x6F;

void require_compressed(const Pubkey& pk) {
    if (!pk.compressed || pk.serialized.size() != 33) {
        throw BtkError("address", "uncompressed key cannot produce p2wpkh or p2tr");
    }
}

const char* hrp_for(Network network) {
    return network == Network::Test ? "tb" : "bc";
}

}  // namespace

bool parse_address_style(const std::string& text, AddressStyle& out) {
    if (text == "p2pkh") {
        out = AddressStyle::P2pkh;
        return true;
    }
    if (text == "p2wpkh") {
        out = AddressStyle::P2wpkh;
        return true;
    }
    if (text == "p2tr") {
        out = AddressStyle::P2tr;
        return true;
    }
    return false;
}

const char* address_style_name(AddressStyle style) {
    switch (style) {
    case AddressStyle::P2pkh:
        return "p2pkh";
    case AddressStyle::P2wpkh:
        return "p2wpkh";
    case AddressStyle::P2tr:
        return "p2tr";
    }
    return "";
}

std::string encode_p2pkh(const Pubkey& pk) {
    if (pk.serialized.empty()) {
        throw BtkError("address", "not a private or public key");
    }
    const Hash160 h = hash160(pk.serialized);
    std::vector<std::uint8_t> payload;
    payload.reserve(21);
    payload.push_back(pk.network == Network::Test ? kP2pkhTest : kP2pkhMain);
    payload.insert(payload.end(), h.begin(), h.end());
    return base58check_encode(payload);
}

std::string encode_p2wpkh(const Pubkey& pk) {
    require_compressed(pk);
    const Hash160 h = hash160(pk.serialized);
    return segwit_encode(hrp_for(pk.network), 0, std::vector<std::uint8_t>(h.begin(), h.end()));
}

std::array<std::uint8_t, 32> taproot_output_x(const Pubkey& pk) {
    require_compressed(pk);

    const std::uint8_t* x = pk.serialized.data() + 1;
    const Hash256 tweak = tagged_hash("TapTweak", x, 32);

    secp256k1_context* ctx = SecpContext::instance().get();
    const bool tweak_zero = std::all_of(tweak.begin(), tweak.end(), [](std::uint8_t b) {
        return b == 0;
    });
    if (!tweak_zero && secp256k1_ec_seckey_verify(ctx, tweak.data()) != 1) {
        throw BtkError("address", "taproot tweak out of range");
    }

    secp256k1_xonly_pubkey internal;
    if (secp256k1_xonly_pubkey_parse(ctx, &internal, x) != 1) {
        throw BtkError("address", "not a private or public key");
    }

    secp256k1_pubkey tweaked;
    if (secp256k1_xonly_pubkey_tweak_add(ctx, &tweaked, &internal, tweak.data()) != 1) {
        throw BtkError("address", "taproot tweak out of range");
    }

    secp256k1_xonly_pubkey out_xonly;
    if (secp256k1_xonly_pubkey_from_pubkey(ctx, &out_xonly, nullptr, &tweaked) != 1) {
        throw BtkError("address", "taproot tweak out of range");
    }

    std::array<std::uint8_t, 32> out{};
    secp256k1_xonly_pubkey_serialize(ctx, out.data(), &out_xonly);
    return out;
}

std::string encode_p2tr(const Pubkey& pk) {
    const auto x = taproot_output_x(pk);
    return segwit_encode(hrp_for(pk.network), 1, std::vector<std::uint8_t>(x.begin(), x.end()));
}

std::string encode_address(const Pubkey& pk, AddressStyle style) {
    switch (style) {
    case AddressStyle::P2pkh:
        return encode_p2pkh(pk);
    case AddressStyle::P2wpkh:
        return encode_p2wpkh(pk);
    case AddressStyle::P2tr:
        return encode_p2tr(pk);
    }
    throw BtkError("address", "unknown address type");
}
