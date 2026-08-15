#include "core/bech32.hpp"
#include "core/hex.hpp"
#include "test/unit/check.hpp"

#include <string>
#include <vector>

namespace {

std::vector<std::uint8_t> seq32() {
    std::vector<std::uint8_t> v(32);
    for (int i = 0; i < 32; ++i) {
        v[static_cast<std::size_t>(i)] = static_cast<std::uint8_t>(i);
    }
    return v;
}

}  // namespace

int main() {
    CHECK(bech32_encode("a", {}, Bech32Variant::Bech32) == "a12uel5l");
    CHECK(bech32_encode("a", {}, Bech32Variant::Bech32m) == "a1lqfn3a");
    CHECK(bech32_encode("abcdef", seq32(), Bech32Variant::Bech32) ==
          "abcdef1qpzry9x8gf2tvdw0s3jn54khce6mua7lmqqqxw");

    std::string hrp0;
    std::vector<std::uint8_t> values0;
    Bech32Variant variant0;
    CHECK(bech32_decode("abcdef1l7aum6echk45nj3s0wdvt2fg8x9yrzpqzd3ryx", hrp0, values0, variant0));
    CHECK(hrp0 == "abcdef");
    CHECK(variant0 == Bech32Variant::Bech32m);
    CHECK(bech32_encode(hrp0, values0, Bech32Variant::Bech32m) ==
          "abcdef1l7aum6echk45nj3s0wdvt2fg8x9yrzpqzd3ryx");

    std::string hrp;
    std::vector<std::uint8_t> values;
    Bech32Variant variant;
    CHECK(bech32_decode("A12UEL5L", hrp, values, variant));
    CHECK(hrp == "a");
    CHECK(values.empty());
    CHECK(variant == Bech32Variant::Bech32);

    CHECK(bech32_decode("A1LQFN3A", hrp, values, variant));
    CHECK(hrp == "a");
    CHECK(values.empty());
    CHECK(variant == Bech32Variant::Bech32m);

    CHECK(!bech32_decode("A12uel5l", hrp, values, variant));

    const std::string p2wpkh_upper = "BC1QW508D6QEJXTDG4Y5R3ZARVARY0C5XW7KV8F3T4";
    const std::string p2wpkh_lower = "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4";
    int witver = -1;
    std::vector<std::uint8_t> program;
    CHECK(segwit_decode(p2wpkh_upper, hrp, witver, program));
    CHECK(hrp == "bc");
    CHECK(witver == 0);
    CHECK(hex_encode(program) == "751e76e8199196d454941c45d1b3a323f1433bd6");
    CHECK(segwit_encode("bc", 0, program) == p2wpkh_lower);

    const std::string untweaked_g = "bc1p0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7vqzk5jj0";
    CHECK(segwit_decode(untweaked_g, hrp, witver, program));
    CHECK(hrp == "bc");
    CHECK(witver == 1);
    CHECK(hex_encode(program) == "79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798");
    CHECK(segwit_encode("bc", 1, program) == untweaked_g);

    std::string hrp2;
    std::vector<std::uint8_t> payload;
    Bech32Variant var;
    CHECK(bech32_decode(p2wpkh_lower, hrp2, payload, var));
    CHECK(var == Bech32Variant::Bech32);
    CHECK(!segwit_decode(bech32_encode(hrp2, payload, Bech32Variant::Bech32m), hrp, witver, program));

    CHECK(bech32_decode(untweaked_g, hrp2, payload, var));
    CHECK(var == Bech32Variant::Bech32m);
    CHECK(!segwit_decode(bech32_encode(hrp2, payload, Bech32Variant::Bech32), hrp, witver, program));

    return 0;
}
