#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

using Hash256 = std::array<std::uint8_t, 32>;
using Hash160 = std::array<std::uint8_t, 20>;

Hash256 sha256(const std::uint8_t* data, std::size_t len);
Hash256 sha256(const std::vector<std::uint8_t>& data);
Hash256 sha256(const std::string& data);

Hash160 ripemd160(const std::uint8_t* data, std::size_t len);
Hash160 ripemd160(const std::vector<std::uint8_t>& data);
Hash160 ripemd160(const std::string& data);

Hash256 hash256(const std::uint8_t* data, std::size_t len);
Hash256 hash256(const std::vector<std::uint8_t>& data);
Hash256 hash256(const std::string& data);

Hash160 hash160(const std::uint8_t* data, std::size_t len);
Hash160 hash160(const std::vector<std::uint8_t>& data);
Hash160 hash160(const std::string& data);

// BIP-340 tagged_hash(tag, msg) = SHA256(SHA256(tag) || SHA256(tag) || msg)
Hash256 tagged_hash(const std::string& tag, const std::uint8_t* msg, std::size_t len);
Hash256 tagged_hash(const std::string& tag, const std::vector<std::uint8_t>& msg);
