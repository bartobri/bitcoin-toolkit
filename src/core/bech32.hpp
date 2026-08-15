#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class Bech32Variant { Bech32, Bech32m };

// Encode 5-bit values (each < 32) with a bech32 / bech32m checksum. HRP must
// be lowercase [33, 126]. Encoder always emits lowercase.
std::string bech32_encode(const std::string& hrp, const std::vector<std::uint8_t>& values,
                          Bech32Variant variant);

// Decode. Accepts all-lower or all-upper; rejects mixed case. On success
// returns lowercase HRP and 5-bit values (checksum stripped).
bool bech32_decode(const std::string& encoded, std::string& hrp, std::vector<std::uint8_t>& values,
                   Bech32Variant& variant);

// SegWit address (BIP-173 / BIP-350). witver 0 uses bech32; witver 1–16 use bech32m.
std::string segwit_encode(const std::string& hrp, int witver, const std::vector<std::uint8_t>& program);
bool segwit_decode(const std::string& addr, std::string& hrp, int& witver,
                   std::vector<std::uint8_t>& program);
