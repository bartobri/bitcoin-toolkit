#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// Extract a standard mainnet address from scriptPubKey. Empty if unrecognized.
std::optional<std::string> address_from_script(const std::vector<std::uint8_t>& script);

// True if s is a mainnet Base58Check or bech32/bech32m address we would index.
bool is_mainnet_address(const std::string& s);

// Style name for a standard mainnet address (p2pkh, p2sh, p2wpkh, p2wsh, p2tr).
// Empty if s is not one of those.
std::optional<std::string> classify_mainnet_address(const std::string& s);
