#include "netstack/header/checksum.hh"

namespace netstack::header {

uint16_t ChecksumCombine(uint16_t a, uint16_t b) {
  const uint32_t v = static_cast<uint32_t>(a) + static_cast<uint32_t>(b);
  return static_cast<uint16_t>(v + (v >> 16));
}

uint16_t Checksum(std::span<const uint8_t> data, uint16_t initial) {
  uint32_t sum = initial;
  size_t i = 0;
  const size_t n = data.size();

  while (i + 1 < n) {
    sum += (static_cast<uint32_t>(data[i]) << 8) + data[i + 1];
    i += 2;
  }
  if (i < n) {
    sum += static_cast<uint32_t>(data[i]) << 8;
  }

  sum = (sum & 0xffff) + (sum >> 16);
  sum = (sum & 0xffff) + (sum >> 16);
  return static_cast<uint16_t>(sum);
}

}  // namespace netstack::header
