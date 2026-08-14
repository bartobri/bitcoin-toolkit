#include "core/base58.hpp"
#include "core/hex.hpp"
#include "test/unit/check.hpp"
#include "util/error.hpp"

int main() {
    // Secret 1, compressed mainnet WIF payload: 0x80 || 32-byte 1 || 0x01
    auto payload = hex_decode(
        "80000000000000000000000000000000000000000000000000000000000000000101");
    CHECK(base58check_encode(payload) == "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn");
    CHECK(hex_encode(base58check_decode(
              "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn")) == hex_encode(payload));

    // Uncompressed mainnet WIF of secret 1
    auto unc = hex_decode("800000000000000000000000000000000000000000000000000000000000000001");
    CHECK(base58check_encode(unc) == "5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreAnchuDf");

    bool threw = false;
    try {
        base58check_decode("KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWo");
    } catch (const BtkError&) {
        threw = true;
    }
    CHECK(threw);
    return 0;
}
