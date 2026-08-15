#pragma once

#include <cstddef>
#include <cstdint>

// Fill buf with n cryptographically random bytes.
// Uses getentropy(2) when available, otherwise /dev/urandom.
// Throws BtkError on short read or open failure.
void random_bytes(std::uint8_t* buf, std::size_t n);
