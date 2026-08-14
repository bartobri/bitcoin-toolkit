#pragma once

#include <cstdint>
#include <string>
#include <vector>

std::string base58_encode(const std::uint8_t* data, std::size_t len);
std::string base58_encode(const std::vector<std::uint8_t>& data);

std::vector<std::uint8_t> base58_decode(const std::string& encoded);

// payload || HASH256(payload)[0..3]
std::string base58check_encode(const std::vector<std::uint8_t>& payload);
std::vector<std::uint8_t> base58check_decode(const std::string& encoded);
