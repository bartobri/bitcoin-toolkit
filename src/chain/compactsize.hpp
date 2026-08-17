#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

void write_compact_size(std::vector<std::uint8_t>& out, std::uint64_t n);
std::uint64_t read_compact_size(const std::uint8_t* data, std::size_t len, std::size_t& off);
