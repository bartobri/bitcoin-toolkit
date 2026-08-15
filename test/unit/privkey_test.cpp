#include "core/hex.hpp"
#include "core/privkey.hpp"
#include "test/unit/check.hpp"
#include "util/error.hpp"

#include <string>

namespace {

void expect_range_error(const std::string& hex) {
    bool threw = false;
    try {
        decode_secret_hex(hex);
    } catch (const BtkError& err) {
        threw = std::string(err.what()).find("out of range") != std::string::npos;
    }
    CHECK(threw);
}

}  // namespace

int main() {
    const std::string secret1 = "0000000000000000000000000000000000000000000000000000000000000001";
    const Secret s1 = decode_secret_hex(secret1);
    CHECK(encode_secret_hex(s1) == secret1);

    Privkey k;
    k.secret = s1;
    k.network = Network::Main;
    k.compressed = true;
    CHECK(encode_wif(k) == "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn");
    k.compressed = false;
    CHECK(encode_wif(k) == "5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreAnchuDf");
    k.network = Network::Test;
    k.compressed = true;
    CHECK(encode_wif(k) == "cMahea7zqjxrtgAbB7LSGbcQUr1uX1ojuat9jZodMN87JcbXMTcA");
    k.compressed = false;
    CHECK(encode_wif(k) == "91avARGdfge8E4tZfYLoxeJ5sGBdNJQH4kvjJoQFacbgwmaKkrx");

    const Privkey g = parse_privkey_string("KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn");
    CHECK(encode_secret_hex(g.secret) == secret1);
    CHECK(g.compressed);
    CHECK(g.network == Network::Main);

    const std::string wiki = "18e14a7b6a307f426a94f8114701e7c8e774e7f9a47e2c2035db29a206321725";
    Privkey w;
    w.secret = decode_secret_hex(wiki);
    w.network = Network::Main;
    w.compressed = true;
    CHECK(encode_wif(w) == "Kx45GeUBSMPReYQwgXiKhG9FzNXrnCeutJp4yjTd5kKxCitadm3C");
    w.compressed = false;
    CHECK(encode_wif(w) == "5J1F7GHadZG3sCCKHCwg8Jvys9xUbFsjLnGec4H125Ny1V9nR6V");
    w.network = Network::Test;
    w.compressed = true;
    CHECK(encode_wif(w) == "cNR4jZU2sR5goytD4wXT4aeKcbqGSekbxLxY69v8aryxTU1SMnJZ");
    w.compressed = false;
    CHECK(encode_wif(w) == "91msh178DnLBqFhbuYqazuUwWpKBkRQvgj8bggdWMp81nVp9PfM");

    const std::string wif_wiki = "0c28fca386c7a227600b2fe50b7cae11ec86d3bf1fbe471be89827e19d72aa1d";
    Privkey ww;
    ww.secret = decode_secret_hex(wif_wiki);
    ww.network = Network::Main;
    ww.compressed = false;
    CHECK(encode_wif(ww) == "5HueCGU8rMjxEXxiPuD5BDku4MkFqeZyd4dZ1jvhTVqvbTLvyTJ");
    ww.compressed = true;
    CHECK(encode_wif(ww) == "KwdMAjGmerYanjeui5SHS7JkmpZvVipYvB2LJGU1ZxJwYvP98617");

    const Hash256 t01 = sha256(std::string("test01"));
    CHECK(hex_encode(t01) == "678e82d907d3e6e71f81d5cf3ddacc3671dc618c38a1b7a9f9393a83d025b296");
    const Privkey from_t01 = privkey_from_digest(t01, Network::Main, true);
    CHECK(encode_wif(from_t01) == "Kzh1d5pXSZLtwsgENakrfCjuGy9txPEb3aEb2y8yyZo65qDs8bTu");

    const Hash256 spp = sha256(std::string("Secret Passphrase"));
    CHECK(hex_encode(spp) == "76ce9bba9487266738e3c4f0b3cfa4be0c0eba52ed1c3c425e06900442efe5e1");
    CHECK(encode_wif(privkey_from_digest(spp, Network::Main, true)) ==
          "L1Cf21MBhiZX9QFTAhN3PGJkyvQzN4CuHwhasHsdV9tkEfiiB8Ug");

    CHECK(encode_secret_dec(s1) == "1");
    CHECK(encode_secret_hex(decode_secret_dec("1")) == secret1);
    CHECK(encode_secret_hex(decode_secret_dec("001")) == secret1);
    CHECK(encode_secret_dec(decode_secret_hex(wiki)) ==
          "11253563012059685825953619222107823549092147699031672238385790369351542642469");
    CHECK(encode_secret_dec(decode_secret_hex(wif_wiki)) ==
          "5500171714335001507730457227127633683517613019341760098818554179534751705629");
    CHECK(encode_secret_hex(parse_privkey_string("1").secret) == secret1);

    bool dec_threw = false;
    try {
        decode_secret_dec("0");
    } catch (const BtkError& err) {
        dec_threw = std::string(err.what()).find("out of range") != std::string::npos;
    }
    CHECK(dec_threw);
    dec_threw = false;
    try {
        decode_secret_dec(
            "115792089237316195423570985008687907852837564279074904382605163141518161494337");
    } catch (const BtkError& err) {
        dec_threw = std::string(err.what()).find("out of range") != std::string::npos;
    }
    CHECK(dec_threw);
    dec_threw = false;
    try {
        decode_secret_dec("+1");
    } catch (const BtkError& err) {
        dec_threw = std::string(err.what()).find("invalid decimal") != std::string::npos;
    }
    CHECK(dec_threw);

    expect_range_error("0000000000000000000000000000000000000000000000000000000000000000");
    expect_range_error("fffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141");
    expect_range_error("fffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364142");

    const Privkey rnd = generate_privkey(Network::Main, true);
    CHECK(secret_is_valid(rnd.secret));
    CHECK(parse_privkey_string(encode_wif(rnd)).secret == rnd.secret);

    bool threw = false;
    try {
        parse_privkey_string("KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWo");
    } catch (const BtkError& err) {
        threw = std::string(err.what()).find("invalid WIF checksum") != std::string::npos;
    }
    CHECK(threw);

    return 0;
}
