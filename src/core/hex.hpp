#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

std::string hex_encode(const std::uint8_t* data, std::size_t len);
std::string hex_encode(const std::vector<std::uint8_t>& data);

template <std::size_t N>
std::string hex_encode(const std::array<std::uint8_t, N>& data) {
    return hex_encode(data.data(), data.size());
}

// Accepts mixed case. Rejects odd length and non-hex. Empty is valid.
std::vector<std::uint8_t> hex_decode(const std::string& hex);
