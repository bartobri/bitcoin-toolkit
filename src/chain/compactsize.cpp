#include "chain/compactsize.hpp"

#include "util/error.hpp"

namespace {

[[noreturn]] void fail() {
    throw BtkError("balance", "invalid compact size");
}

}  // namespace

void write_compact_size(std::vector<std::uint8_t>& out, std::uint64_t n) {
    if (n < 0xfd) {
        out.push_back(static_cast<std::uint8_t>(n));
    } else if (n <= 0xffff) {
        out.push_back(0xfd);
        out.push_back(static_cast<std::uint8_t>(n));
        out.push_back(static_cast<std::uint8_t>(n >> 8));
    } else if (n <= 0xffffffffu) {
        out.push_back(0xfe);
        out.push_back(static_cast<std::uint8_t>(n));
        out.push_back(static_cast<std::uint8_t>(n >> 8));
        out.push_back(static_cast<std::uint8_t>(n >> 16));
        out.push_back(static_cast<std::uint8_t>(n >> 24));
    } else {
        out.push_back(0xff);
        for (int i = 0; i < 8; ++i) {
            out.push_back(static_cast<std::uint8_t>(n >> (8 * i)));
        }
    }
}

std::uint64_t read_compact_size(const std::uint8_t* data, std::size_t len, std::size_t& off) {
    if (off >= len) {
        fail();
    }
    const std::uint8_t b = data[off++];
    if (b < 0xfd) {
        return b;
    }
    if (b == 0xfd) {
        if (off + 2 > len) {
            fail();
        }
        const std::uint64_t n = static_cast<std::uint64_t>(data[off]) |
                                (static_cast<std::uint64_t>(data[off + 1]) << 8);
        off += 2;
        if (n < 0xfd) {
            fail();
        }
        return n;
    }
    if (b == 0xfe) {
        if (off + 4 > len) {
            fail();
        }
        const std::uint64_t n = static_cast<std::uint64_t>(data[off]) |
                                (static_cast<std::uint64_t>(data[off + 1]) << 8) |
                                (static_cast<std::uint64_t>(data[off + 2]) << 16) |
                                (static_cast<std::uint64_t>(data[off + 3]) << 24);
        off += 4;
        if (n <= 0xffff) {
            fail();
        }
        return n;
    }
    if (off + 8 > len) {
        fail();
    }
    std::uint64_t n = 0;
    for (int i = 0; i < 8; ++i) {
        n |= static_cast<std::uint64_t>(data[off + static_cast<std::size_t>(i)]) << (8 * i);
    }
    off += 8;
    if (n <= 0xffffffffu) {
        fail();
    }
    return n;
}
