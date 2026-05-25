#include "utils.hh"

#include <cstddef>
#include <cstdint>

namespace {

bool IsValidHeaderLength(std::size_t header_len) {
  return header_len >= 20 && header_len % 4 == 0;
}

void FoldCarries(uint32_t& sum) {
  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }
}

uint32_t Sum16BitWords(const uint8_t* header, std::size_t header_len,
                       bool skip_checksum_field) {
  uint32_t sum = 0;
  for (std::size_t i = 0; i + 1 < header_len; i += 2) {
    if (skip_checksum_field && i == 10) {
      continue;
    }
    sum += ReadBE16(header + i);
  }
  FoldCarries(sum);
  return sum;
}

}  // namespace

uint16_t ReadBE16(const uint8_t* data) {
  return (static_cast<uint16_t>(data[0]) << 8) | static_cast<uint16_t>(data[1]);
}

uint32_t ReadBE32(const uint8_t* data) {
  return (static_cast<uint32_t>(data[0]) << 24) |
         (static_cast<uint32_t>(data[1]) << 16) |
         (static_cast<uint32_t>(data[2]) << 8) | static_cast<uint32_t>(data[3]);
}

uint16_t WriteBE16(uint8_t* data, uint16_t value) {
  data[0] = static_cast<uint8_t>(value >> 8);
  data[1] = static_cast<uint8_t>(value);
  return 2;
}

uint32_t WriteBE32(uint8_t* data, uint32_t value) {
  data[0] = static_cast<uint8_t>(value >> 24);
  data[1] = static_cast<uint8_t>(value >> 16);
  data[2] = static_cast<uint8_t>(value >> 8);
  data[3] = static_cast<uint8_t>(value);
  return 4;
}

std::optional<uint16_t> CalculateChecksum(const uint8_t* header,
                                          std::size_t header_len) {
  if (!IsValidHeaderLength(header_len)) {
    return std::nullopt;
  }

  const uint32_t sum =
      Sum16BitWords(header, header_len, /*skip_checksum_field=*/true);
  return static_cast<uint16_t>(~sum);
}

bool VerifyChecksum(const uint8_t* header, std::size_t header_len) {
  if (!IsValidHeaderLength(header_len)) {
    return false;
  }

  const uint32_t sum =
      Sum16BitWords(header, header_len, /*skip_checksum_field=*/false);
  return sum == 0xFFFF;
}
