/**
 * @file checksum.cc
 * @brief RFC 1071 Internet 校验和实现（header 模块，无状态）。
 */

#include "netstack/header/checksum.hh"

namespace netstack::header {

uint16_t ChecksumCombine(uint16_t a, uint16_t b) {
  const uint32_t v = static_cast<uint32_t>(a) + static_cast<uint32_t>(b);
  // 进位加回低 16 位（ones' complement 折叠）
  return static_cast<uint16_t>(v + (v >> 16));
}

uint16_t Checksum(std::span<const uint8_t> data, uint16_t initial) {
  uint32_t sum = initial;
  size_t i = 0;
  const size_t n = data.size();

  // 16 位一组累加（大端：高字节在前）
  while (i + 1 < n) {
    sum += (static_cast<uint32_t>(data[i]) << 8) + data[i + 1];
    i += 2;
  }
  if (i < n) {
    // 奇数长度：最后一个字节放在 16 位字的高 8 位
    sum += static_cast<uint32_t>(data[i]) << 8;
  }

  sum = (sum & 0xffff) + (sum >> 16);
  sum = (sum & 0xffff) + (sum >> 16);
  return static_cast<uint16_t>(sum);
}

}  // namespace netstack::header
