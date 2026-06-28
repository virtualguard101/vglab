/**
 * @file checksum.cc
 * @brief RFC 1071 Internet 校验和实现（header 模块，无状态）。
 *
 * ## 算法要点
 *
 * - **Internet checksum** 使用 16 位 one's complement 算术：累加时产生的进位要
 *   「折回」到低 16 位，而不是丢弃。
 * - 按**大端** 16 位字读取；奇数尾字节放在 16 位字的高 8 位。
 *
 * ## 在 IPv4 中的用法
 *
 * - **发送方**：校验和字段先写 0，对整头求 `Checksum()`，再写入 `~sum`。
 * - **接收方**：对**含校验和字段的整头**再算一遍，折叠结果应为 `0xFFFF`
 *   （即 `IsChecksumValid()`）。
 *
 * ## ChecksumCombine
 *
 * 用于增量更新（如 `EncodePartial`）：把已有部分和与新区间合并时仍需折回进位。
 *
 * @see RFC 1071
 * @see include/netstack/header/checksum.hh
 * @see docs/m0.md
 */

#include "netstack/header/checksum.hh"

namespace netstack::header {

/**
 * @brief 将两个 16 位部分和合并，并处理 32 位加法产生的进位。
 *
 * 示例：a=0xFFFF, b=0x0001 → v=0x10000 → 折回 → 0x0001（非简单 uint16 截断）。
 */
uint16_t ChecksumCombine(uint16_t a, uint16_t b) {
  const uint32_t v = static_cast<uint32_t>(a) + static_cast<uint32_t>(b);
  // 进位加回低 16 位（ones' complement 折叠）
  return static_cast<uint16_t>(v + (v >> 16));
}

/**
 * @brief 对 data 做 RFC 1071 累加；返回值为**未取反**的 16 位和。
 *
 * 实现与 references/tcpip/header/checksum.go 一致：
 * - 按大端 16 位字读取；
 * - 奇数尾字节放在 16 位字的高 8 位（低 8 位为 0）；
 * - 最后两次折叠，保证 sum 的高 16 位为 0。
 */
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
