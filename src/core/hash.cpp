#include "core/hash.hpp"

#include <cstring>

namespace {

constexpr std::uint32_t rotr32(std::uint32_t x, int n) {
    return (x >> n) | (x << (32 - n));
}

constexpr std::uint32_t rotl32(std::uint32_t x, int n) {
    return (x << n) | (x >> (32 - n));
}

std::uint32_t load_be32(const std::uint8_t* p) {
    return (static_cast<std::uint32_t>(p[0]) << 24) |
           (static_cast<std::uint32_t>(p[1]) << 16) |
           (static_cast<std::uint32_t>(p[2]) << 8) |
           (static_cast<std::uint32_t>(p[3]));
}

void store_be32(std::uint8_t* p, std::uint32_t v) {
    p[0] = static_cast<std::uint8_t>(v >> 24);
    p[1] = static_cast<std::uint8_t>(v >> 16);
    p[2] = static_cast<std::uint8_t>(v >> 8);
    p[3] = static_cast<std::uint8_t>(v);
}

std::uint32_t load_le32(const std::uint8_t* p) {
    return static_cast<std::uint32_t>(p[0]) |
           (static_cast<std::uint32_t>(p[1]) << 8) |
           (static_cast<std::uint32_t>(p[2]) << 16) |
           (static_cast<std::uint32_t>(p[3]) << 24);
}

void store_le32(std::uint8_t* p, std::uint32_t v) {
    p[0] = static_cast<std::uint8_t>(v);
    p[1] = static_cast<std::uint8_t>(v >> 8);
    p[2] = static_cast<std::uint8_t>(v >> 16);
    p[3] = static_cast<std::uint8_t>(v >> 24);
}

const std::uint32_t kSha256[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4,
    0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe,
    0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f,
    0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
    0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116,
    0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7,
    0xc67178f2};

void sha256_compress(std::uint32_t h[8], const std::uint8_t block[64]) {
    std::uint32_t w[64];
    for (int i = 0; i < 16; ++i) {
        w[i] = load_be32(block + i * 4);
    }
    for (int i = 16; i < 64; ++i) {
        const std::uint32_t s0 = rotr32(w[i - 15], 7) ^ rotr32(w[i - 15], 18) ^ (w[i - 15] >> 3);
        const std::uint32_t s1 = rotr32(w[i - 2], 17) ^ rotr32(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    std::uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
    std::uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];
    for (int i = 0; i < 64; ++i) {
        const std::uint32_t S1 = rotr32(e, 6) ^ rotr32(e, 11) ^ rotr32(e, 25);
        const std::uint32_t ch = (e & f) ^ ((~e) & g);
        const std::uint32_t t1 = hh + S1 + ch + kSha256[i] + w[i];
        const std::uint32_t S0 = rotr32(a, 2) ^ rotr32(a, 13) ^ rotr32(a, 22);
        const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        const std::uint32_t t2 = S0 + maj;
        hh = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }
    h[0] += a;
    h[1] += b;
    h[2] += c;
    h[3] += d;
    h[4] += e;
    h[5] += f;
    h[6] += g;
    h[7] += hh;
}

std::uint32_t f1(std::uint32_t x, std::uint32_t y, std::uint32_t z) { return x ^ y ^ z; }
std::uint32_t f2(std::uint32_t x, std::uint32_t y, std::uint32_t z) { return (x & y) | (~x & z); }
std::uint32_t f3(std::uint32_t x, std::uint32_t y, std::uint32_t z) { return (x | ~y) ^ z; }
std::uint32_t f4(std::uint32_t x, std::uint32_t y, std::uint32_t z) { return (x & z) | (y & ~z); }
std::uint32_t f5(std::uint32_t x, std::uint32_t y, std::uint32_t z) { return x ^ (y | ~z); }

const int rmd_r[80] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                       7,  4,  13, 1,  10, 6,  15, 3,  12, 0,  9,  5,  2,  14, 11, 8,
                       3,  10, 14, 4,  9,  15, 8,  1,  2,  7,  0,  6,  13, 11, 5,  12,
                       1,  9,  11, 10, 0,  8,  12, 4,  13, 3,  7,  15, 14, 5,  6,  2,
                       4,  0,  5,  9,  7,  12, 2,  10, 14, 1,  3,  8,  11, 6,  15, 13};

const int rmd_rr[80] = {5,  14, 7,  0,  9,  2,  11, 4,  13, 6,  15, 8,  1,  10, 3,  12,
                        6,  11, 3,  7,  0,  13, 5,  10, 14, 15, 8,  12, 4,  9,  1,  2,
                        15, 5,  1,  3,  7,  14, 6,  9,  11, 8,  12, 2,  10, 0,  4,  13,
                        8,  6,  4,  1,  3,  11, 15, 0,  5,  12, 2,  13, 9,  7,  10, 14,
                        12, 15, 10, 4,  1,  5,  8,  7,  6,  2,  13, 14, 0,  3,  9,  11};

const int rmd_s[80] = {11, 14, 15, 12, 5,  8,  7,  9,  11, 13, 14, 15, 6,  7,  9,  8,
                       7,  6,  8,  13, 11, 9,  7,  15, 7,  12, 15, 9,  11, 7,  13, 12,
                       11, 13, 6,  7,  14, 9,  13, 15, 14, 8,  13, 6,  5,  12, 7,  5,
                       11, 12, 14, 15, 14, 15, 9,  8,  9,  14, 5,  6,  8,  6,  5,  12,
                       9,  15, 5,  11, 6,  8,  13, 12, 5,  12, 13, 14, 11, 8,  5,  6};

const int rmd_ss[80] = {8,  9,  9,  11, 13, 15, 15, 5,  7,  7,  8,  11, 14, 14, 12, 6,
                        9,  13, 15, 7,  12, 8,  9,  11, 7,  7,  12, 7,  6,  15, 13, 11,
                        9,  7,  15, 11, 8,  6,  6,  14, 12, 13, 5,  14, 13, 13, 7,  5,
                        15, 5,  8,  11, 14, 14, 6,  14, 6,  9,  12, 9,  12, 5,  15, 8,
                        8,  5,  12, 9,  12, 5,  14, 6,  8,  13, 6,  5,  15, 13, 11, 11};

void rmd160_compress(std::uint32_t h[5], const std::uint8_t block[64]) {
    std::uint32_t x[16];
    for (int i = 0; i < 16; ++i) {
        x[i] = load_le32(block + i * 4);
    }

    std::uint32_t al = h[0], bl = h[1], cl = h[2], dl = h[3], el = h[4];
    std::uint32_t ar = h[0], br = h[1], cr = h[2], dr = h[3], er = h[4];

    for (int i = 0; i < 80; ++i) {
        std::uint32_t f;
        std::uint32_t k;
        if (i < 16) {
            f = f1(bl, cl, dl);
            k = 0x00000000;
        } else if (i < 32) {
            f = f2(bl, cl, dl);
            k = 0x5a827999;
        } else if (i < 48) {
            f = f3(bl, cl, dl);
            k = 0x6ed9eba1;
        } else if (i < 64) {
            f = f4(bl, cl, dl);
            k = 0x8f1bbcdc;
        } else {
            f = f5(bl, cl, dl);
            k = 0xa953fd4e;
        }
        std::uint32_t t = rotl32(al + f + x[rmd_r[i]] + k, rmd_s[i]) + el;
        al = el;
        el = dl;
        dl = rotl32(cl, 10);
        cl = bl;
        bl = t;

        if (i < 16) {
            f = f5(br, cr, dr);
            k = 0x50a28be6;
        } else if (i < 32) {
            f = f4(br, cr, dr);
            k = 0x5c4dd124;
        } else if (i < 48) {
            f = f3(br, cr, dr);
            k = 0x6d703ef3;
        } else if (i < 64) {
            f = f2(br, cr, dr);
            k = 0x7a6d76e9;
        } else {
            f = f1(br, cr, dr);
            k = 0x00000000;
        }
        t = rotl32(ar + f + x[rmd_rr[i]] + k, rmd_ss[i]) + er;
        ar = er;
        er = dr;
        dr = rotl32(cr, 10);
        cr = br;
        br = t;
    }

    const std::uint32_t t = h[1] + cl + dr;
    h[1] = h[2] + dl + er;
    h[2] = h[3] + el + ar;
    h[3] = h[4] + al + br;
    h[4] = h[0] + bl + cr;
    h[0] = t;
}

template <typename Compress, typename StoreWord, std::size_t DigestWords>
void hash_padded(const std::uint8_t* data,
                 std::size_t len,
                 std::uint32_t* state,
                 Compress compress,
                 StoreWord store_word,
                 bool length_be,
                 std::uint8_t* out) {
    std::uint8_t block[64];
    std::size_t offset = 0;
    while (offset + 64 <= len) {
        compress(state, data + offset);
        offset += 64;
    }

    std::size_t rem = len - offset;
    std::memset(block, 0, 64);
    if (rem) {
        std::memcpy(block, data + offset, rem);
    }
    block[rem] = 0x80;

    if (rem >= 56) {
        compress(state, block);
        std::memset(block, 0, 64);
    }

    const std::uint64_t bits = static_cast<std::uint64_t>(len) * 8;
    if (length_be) {
        store_be32(block + 56, static_cast<std::uint32_t>(bits >> 32));
        store_be32(block + 60, static_cast<std::uint32_t>(bits));
    } else {
        store_le32(block + 56, static_cast<std::uint32_t>(bits));
        store_le32(block + 60, static_cast<std::uint32_t>(bits >> 32));
    }
    compress(state, block);

    for (std::size_t i = 0; i < DigestWords; ++i) {
        store_word(out + i * 4, state[i]);
    }
}

}  // namespace

Hash256 sha256(const std::uint8_t* data, std::size_t len) {
    std::uint32_t h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                          0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    Hash256 out{};
    hash_padded<decltype(&sha256_compress), decltype(&store_be32), 8>(
        data, len, h, sha256_compress, store_be32, true, out.data());
    return out;
}

Hash256 sha256(const std::vector<std::uint8_t>& data) {
    return sha256(data.data(), data.size());
}

Hash256 sha256(const std::string& data) {
    return sha256(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());
}

Hash160 ripemd160(const std::uint8_t* data, std::size_t len) {
    std::uint32_t h[5] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0};
    Hash160 out{};
    hash_padded<decltype(&rmd160_compress), decltype(&store_le32), 5>(
        data, len, h, rmd160_compress, store_le32, false, out.data());
    return out;
}

Hash160 ripemd160(const std::vector<std::uint8_t>& data) {
    return ripemd160(data.data(), data.size());
}

Hash160 ripemd160(const std::string& data) {
    return ripemd160(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());
}

Hash256 hash256(const std::uint8_t* data, std::size_t len) {
    const Hash256 first = sha256(data, len);
    return sha256(first.data(), first.size());
}

Hash256 hash256(const std::vector<std::uint8_t>& data) {
    return hash256(data.data(), data.size());
}

Hash256 hash256(const std::string& data) {
    return hash256(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());
}

Hash160 hash160(const std::uint8_t* data, std::size_t len) {
    const Hash256 first = sha256(data, len);
    return ripemd160(first.data(), first.size());
}

Hash160 hash160(const std::vector<std::uint8_t>& data) {
    return hash160(data.data(), data.size());
}

Hash160 hash160(const std::string& data) {
    return hash160(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());
}

Hash256 tagged_hash(const std::string& tag, const std::uint8_t* msg, std::size_t len) {
    const Hash256 th = sha256(tag);
    std::vector<std::uint8_t> buf;
    buf.reserve(64 + len);
    buf.insert(buf.end(), th.begin(), th.end());
    buf.insert(buf.end(), th.begin(), th.end());
    buf.insert(buf.end(), msg, msg + len);
    return sha256(buf);
}

Hash256 tagged_hash(const std::string& tag, const std::vector<std::uint8_t>& msg) {
    return tagged_hash(tag, msg.data(), msg.size());
}
