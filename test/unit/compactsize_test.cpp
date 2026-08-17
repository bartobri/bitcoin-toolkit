#include "chain/compactsize.hpp"
#include "core/hex.hpp"
#include "test/unit/check.hpp"

#include <string>
#include <utility>
#include <vector>

namespace {

void expect_cs(std::uint64_t n, const char* hex) {
    std::vector<std::uint8_t> out;
    write_compact_size(out, n);
    CHECK(hex_encode(out) == hex);
    std::size_t off = 0;
    CHECK(read_compact_size(out.data(), out.size(), off) == n);
    CHECK(off == out.size());
}

}  // namespace

int main() {
    expect_cs(0, "00");
    expect_cs(1, "01");
    expect_cs(23, "17");
    expect_cs(127, "7f");
    expect_cs(128, "80");
    expect_cs(200, "c8");
    expect_cs(252, "fc");
    expect_cs(253, "fdfd00");
    expect_cs(255, "fdff00");
    expect_cs(256, "fd0001");
    return 0;
}
