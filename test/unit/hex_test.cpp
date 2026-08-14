#include "core/hex.hpp"
#include "test/unit/check.hpp"
#include "util/error.hpp"

int main() {
    CHECK(hex_encode(hex_decode("00ff")) == "00ff");
    CHECK(hex_encode(hex_decode("AbCd")) == "abcd");
    CHECK(hex_decode("").empty());

    bool threw = false;
    try {
        hex_decode("abc");
    } catch (const BtkError&) {
        threw = true;
    }
    CHECK(threw);

    threw = false;
    try {
        hex_decode("zz");
    } catch (const BtkError&) {
        threw = true;
    }
    CHECK(threw);
    return 0;
}
