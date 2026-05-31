#pragma once

#include <cstdint>
#include <span>

namespace netstack::header {

/// RFC 1071 Internet checksum over \p data, folded with \p initial.
uint16_t Checksum(std::span<const uint8_t> data, uint16_t initial = 0);

/// Combines two partial 16-bit sums (references/header.ChecksumCombine).
uint16_t ChecksumCombine(uint16_t a, uint16_t b);

}  // namespace netstack::header
