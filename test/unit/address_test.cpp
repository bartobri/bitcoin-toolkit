#include "core/address.hpp"
#include "core/hex.hpp"
#include "core/privkey.hpp"
#include "core/pubkey.hpp"
#include "test/unit/check.hpp"
#include "util/error.hpp"

#include <string>

namespace {

Pubkey from_hex_secret(const std::string& hex, bool compressed, Network network) {
    return pubkey_from_secret(decode_secret_hex(hex), compressed, network);
}

void expect_uncompressed(AddressStyle style, const Pubkey& pk) {
    bool threw = false;
    try {
        encode_address(pk, style);
    } catch (const BtkError& err) {
        threw = std::string(err.what()) == "uncompressed key cannot produce p2wpkh or p2tr";
    }
    CHECK(threw);
}

}  // namespace

int main() {
    const std::string secret1 = "0000000000000000000000000000000000000000000000000000000000000001";
    const std::string secret6 = "0000000000000000000000000000000000000000000000000000000000000006";
    const std::string wiki = "18e14a7b6a307f426a94f8114701e7c8e774e7f9a47e2c2035db29a206321725";
    const std::string a6 = "6b973d88838f27366ed61c9ad6367663045cb456e28335c109e30717ae0c6baa";

    const Pubkey g = from_hex_secret(secret1, true, Network::Main);
    const Pubkey g_unc = from_hex_secret(secret1, false, Network::Main);
    const Pubkey g_test = from_hex_secret(secret1, true, Network::Test);
    const Pubkey g_unc_test = from_hex_secret(secret1, false, Network::Test);
    const Pubkey s6 = from_hex_secret(secret6, true, Network::Main);

    CHECK(encode_p2pkh(g) == "1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH");
    CHECK(encode_p2pkh(g_unc) == "1EHNa6Q4Jz2uvNExL497mE43ikXhwF6kZm");
    CHECK(encode_p2pkh(g_test) == "mrCDrCybB6J1vRfbwM5hemdJz73FwDBC8r");
    CHECK(encode_p2pkh(g_unc_test) == "mtoKs9V381UAhUia3d7Vb9GNak8Qvmcsme");

    CHECK(encode_p2wpkh(g) == "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4");
    CHECK(encode_p2wpkh(g_test) == "tb1qw508d6qejxtdg4y5r3zarvary0c5xw7kxpjzsx");

    CHECK(hex_encode(taproot_output_x(g)) ==
          "da4710964f7852695de2da025290e24af6d8c281de5a0b902b7135fd9fd74d21");
    CHECK(encode_p2tr(g) == "bc1pmfr3p9j00pfxjh0zmgp99y8zftmd3s5pmedqhyptwy6lm87hf5sspknck9");
    CHECK(encode_p2tr(g_test) == "tb1pmfr3p9j00pfxjh0zmgp99y8zftmd3s5pmedqhyptwy6lm87hf5ssk79hv2");

    CHECK(encode_p2pkh(s6) == "1Cf2hs39Woi61YNkYGUAcohL2K2q4pawBq");
    CHECK(encode_p2wpkh(s6) == "bc1q0ldfeupqc9k2eaffep7cm6yml3ct3jwtwzqt7k");
    CHECK(hex_encode(taproot_output_x(s6)) ==
          "a8e1f6946495d797bda3c3c6a88cf34375130c57a42a966c9a0508bf3cc2fc1a");
    CHECK(encode_p2tr(s6) == "bc1p4rsld9ryjhte00drc0r23r8ngd63xrzh5s4fvmy6q5yt70xzlsdqcuvtzv");
    Pubkey s6_test = s6;
    s6_test.network = Network::Test;
    CHECK(encode_p2tr(s6_test) == "tb1p4rsld9ryjhte00drc0r23r8ngd63xrzh5s4fvmy6q5yt70xzlsdq056ycr");

    // Same X, even Y vs odd Y → same P2TR.
    const std::string even_same_x =
        "02fff97bd5755eeea420453a14355235d382f6472f8568a18b2f057a1460297556";
    CHECK(encode_p2tr(parse_pubkey_hex(even_same_x)) == encode_p2tr(s6));

    const Pubkey wiki_unc = from_hex_secret(wiki, false, Network::Main);
    const Pubkey wiki_comp = from_hex_secret(wiki, true, Network::Main);
    CHECK(encode_p2pkh(wiki_unc) == "16UwLL9Risc3QfPqBUvKofHmBQ7wMtjvM");
    CHECK(encode_p2pkh(wiki_comp) == "1PMycacnJaSqwwJqjawXBErnLsZ7RkXUAs");
    CHECK(encode_p2wpkh(wiki_comp) == "bc1q7499s50fxu4c0qg23esvm5h8elvqkm33r2tdza");
    CHECK(hex_encode(taproot_output_x(wiki_comp)) ==
          "a137269b60cc269ca2aaf9257d5554f3e660f9a80bf82ba95c05451465c52137");
    CHECK(encode_p2tr(wiki_comp) ==
          "bc1p5ymjdxmqesnfeg42lyjh642570nxp7dgp0uzh22uq4z3gew9yymst6pshk");

    const Pubkey a6pk = from_hex_secret(a6, true, Network::Main);
    CHECK(hex_encode(taproot_output_x(a6pk)) ==
          "53a1f6e454df1aa2776a2814a721372d6258050de330b3c6d10ee8f4e0dda343");
    CHECK(encode_p2tr(a6pk) == "bc1p2wsldez5mud2yam29q22wgfh9439spgduvct83k3pm50fcxa5dps59h4z5");

    expect_uncompressed(AddressStyle::P2wpkh, g_unc);
    expect_uncompressed(AddressStyle::P2tr, g_unc);

    AddressStyle style;
    CHECK(parse_address_style("p2pkh", style) && style == AddressStyle::P2pkh);
    CHECK(parse_address_style("p2wpkh", style) && style == AddressStyle::P2wpkh);
    CHECK(parse_address_style("p2tr", style) && style == AddressStyle::P2tr);
    CHECK(!parse_address_style("bech32m", style));
    CHECK(std::string(address_style_name(AddressStyle::P2pkh)) == "p2pkh");

    return 0;
}
