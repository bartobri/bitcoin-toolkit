#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "core/hash.hpp"
#include "core/network.hpp"

using Secret = std::array<std::uint8_t, 32>;

struct Privkey {
    Secret secret{};
    Network network = Network::Main;
    bool compressed = true;
};

bool secret_is_valid(const Secret& secret);

// Throws "private key out of range" if secret is 0 or >= n.
void require_valid_secret(const Secret& secret);

std::string encode_wif(const Privkey& key);
Privkey decode_wif(const std::string& wif);

std::string encode_secret_hex(const Secret& secret);
Secret decode_secret_hex(const std::string& hex);

std::string encode_secret_dec(const Secret& secret);
Secret decode_secret_dec(const std::string& dec);

bool looks_like_decimal(const std::string& text);

// True if text is a 37/38-byte base58 payload with a WIF version or a bad
// checksum (so a typo’d WIF is not guessed as something else).
bool looks_like_wif(const std::string& text);

// True if text is WIF-shaped, 64-char hex, or decimal digits (including
// values that will then fail checksum / range checks).
bool looks_like_privkey_text(const std::string& text);

// Bare-string guess: WIF, then 64-char hex, then decimal digits.
Privkey parse_privkey_string(const std::string& text);

Privkey generate_privkey(Network network, bool compressed);

// SHA-256(bytes) as the secret. Rejects out-of-range hashes.
Privkey privkey_from_digest(const Hash256& digest, Network network, bool compressed);
