#include "core/hash.hpp"
#include "core/hex.hpp"
#include "test/unit/check.hpp"

#include <string>
#include <vector>

int main() {
    CHECK(hex_encode(sha256(std::string("abc"))) ==
          "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    CHECK(hex_encode(ripemd160(std::string("abc"))) == "8eb208f7e05d987a9b044a8e98c6b087f15a0bfc");
    CHECK(hex_encode(hash256(std::string("abc"))) ==
          "4f8b42c22dd3729b519ba6f68d2da7cc5b2d606d05daed5ad5128cc03e6c6358");
    CHECK(hex_encode(hash160(std::string("abc"))) == "bb1be98c142444d7a56aa3981c3942a978e4dc33");
    CHECK(hex_encode(hash256(std::string(""))) ==
          "5df6e0e2761359d30a8275058e299fcc0381534545f55cf43e41983f5d4c9456");

    const auto g_x = hex_decode("79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798");
    CHECK(hex_encode(tagged_hash("TapTweak", g_x)) ==
          "3cf5216d476a5e637bf0da674e50ddf55c403270dd36494dfcca438132fa30e7");

    const auto g_pub = hex_decode("0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798");
    CHECK(hex_encode(hash160(g_pub)) == "751e76e8199196d454941c45d1b3a323f1433bd6");

    return 0;
}
