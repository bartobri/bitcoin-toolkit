#include "core/hex.hpp"
#include "core/privkey.hpp"
#include "core/pubkey.hpp"
#include "test/unit/check.hpp"
#include "util/error.hpp"

#include <string>

namespace {

void expect_not_a_key(const std::string& hex) {
    bool threw = false;
    try {
        parse_pubkey_hex(hex);
    } catch (const BtkError& err) {
        threw = std::string(err.what()) == "not a private or public key";
    }
    CHECK(threw);
}

}  // namespace

int main() {
    const std::string secret1 = "0000000000000000000000000000000000000000000000000000000000000001";
    const Secret s1 = decode_secret_hex(secret1);
    const std::string g_comp = "0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798";
    const std::string g_unc =
        "0479be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798483ada7726a3c4655da4fbfc0e1"
        "108a8fd17b448a68554199c47d08ffb10d4b8";

    const Pubkey g = pubkey_from_secret(s1, true, Network::Main);
    CHECK(encode_pubkey_hex(g) == g_comp);
    CHECK(g.compressed);
    CHECK(g.serialized.size() == 33);

    const Pubkey g2 = pubkey_from_secret(s1, false, Network::Main);
    CHECK(encode_pubkey_hex(g2) == g_unc);
    CHECK(!g2.compressed);
    CHECK(g2.serialized.size() == 65);

    CHECK(encode_pubkey_hex(parse_pubkey_hex(g_comp)) == g_comp);
    CHECK(encode_pubkey_hex(parse_pubkey_hex(g_unc)) == g_unc);
    CHECK(encode_pubkey_hex(parse_pubkey_hex("0279BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F"
                                            "2815B16F81798")) == g_comp);

    CHECK(encode_pubkey_hex(recompress_pubkey(parse_pubkey_hex(g_unc), true)) == g_comp);
    CHECK(encode_pubkey_hex(recompress_pubkey(parse_pubkey_hex(g_comp), false)) == g_unc);

    const std::string wiki = "18e14a7b6a307f426a94f8114701e7c8e774e7f9a47e2c2035db29a206321725";
    const Pubkey w = pubkey_from_secret(decode_secret_hex(wiki), true, Network::Main);
    CHECK(encode_pubkey_hex(w) ==
          "0250863ad64a87ae8a2fe83c1af1a8403cb53f53e486d8511dad8a04887e5b2352");
    CHECK(encode_pubkey_hex(pubkey_from_secret(decode_secret_hex(wiki), false, Network::Main)) ==
          "0450863ad64a87ae8a2fe83c1af1a8403cb53f53e486d8511dad8a04887e5b23522cd470243453a299fa9e77"
          "237716103abc11a1df38855ed6f2ee187e9c582ba6");

    const std::string secret6 = "0000000000000000000000000000000000000000000000000000000000000006";
    CHECK(encode_pubkey_hex(pubkey_from_secret(decode_secret_hex(secret6), true, Network::Main)) ==
          "03fff97bd5755eeea420453a14355235d382f6472f8568a18b2f057a1460297556");

    const std::string wif_wiki = "0c28fca386c7a227600b2fe50b7cae11ec86d3bf1fbe471be89827e19d72aa1d";
    CHECK(encode_pubkey_hex(pubkey_from_secret(decode_secret_hex(wif_wiki), true, Network::Main)) ==
          "02d0de0aaeaefad02b8bdc8a01a1b8b11c696bd3d66a2c5f10780d95b7df42645c");

    CHECK(looks_like_pubkey_hex(g_comp));
    CHECK(looks_like_pubkey_hex(g_unc));
    CHECK(!looks_like_pubkey_hex(secret1));
    CHECK(!looks_like_pubkey_hex("02"));
    CHECK(!looks_like_pubkey_hex("zz79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798"));

    expect_not_a_key(secret1);
    expect_not_a_key("02ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

    bool range = false;
    try {
        Secret z{};
        pubkey_from_secret(z, true, Network::Main);
    } catch (const BtkError& err) {
        range = std::string(err.what()) == "private key out of range";
    }
    CHECK(range);

    return 0;
}
