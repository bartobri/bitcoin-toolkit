#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// Extract a standard mainnet address from scriptPubKey. Empty if unrecognized.
std::optional<std::string> address_from_script(const std::vector<std::uint8_t>& script);

// True if s is a mainnet Base58Check or bech32/bech32m address we would index.
bool is_mainnet_address(const std::string& s);
