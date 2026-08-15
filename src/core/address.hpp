#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "core/pubkey.hpp"

enum class AddressStyle { P2pkh, P2wpkh, P2tr };

bool parse_address_style(const std::string& text, AddressStyle& out);
const char* address_style_name(AddressStyle style);

std::string encode_p2pkh(const Pubkey& pk);
std::string encode_p2wpkh(const Pubkey& pk);
std::string encode_p2tr(const Pubkey& pk);
std::string encode_address(const Pubkey& pk, AddressStyle style);

// BIP-341 empty-tree output key x(Q). Requires a compressed pubkey.
std::array<std::uint8_t, 32> taproot_output_x(const Pubkey& pk);
