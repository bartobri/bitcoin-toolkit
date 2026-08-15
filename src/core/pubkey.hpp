#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "core/network.hpp"
#include "core/privkey.hpp"

struct Pubkey {
    std::vector<std::uint8_t> serialized;
    Network network = Network::Main;
    bool compressed = true;
};

bool looks_like_pubkey_hex(const std::string& text);

Pubkey pubkey_from_secret(const Secret& secret, bool compressed, Network network);
Pubkey parse_pubkey_hex(const std::string& hex);
Pubkey recompress_pubkey(const Pubkey& pk, bool compressed);

std::string encode_pubkey_hex(const Pubkey& pk);
