/**
 * @file udp.hh
 * @brief UDP 报文头视图与编解码（对标 references/tcpip/header/udp.go）。
 *
 * ## UDP 头布局（RFC 768，固定 8 字节）
 *
 * @code
 *  0      7 8     15 16    23 24    31
 * +--------+--------+--------+--------+
 * |   Source Port   | Destination Port|
 * +--------+--------+--------+--------+
 * |     Length      |    Checksum     |
 * +--------+--------+--------+--------+
 * |          Payload (变长)            |
 * @endcode
 *
 * - **Length**：整个 UDP 段长度（头 + 载荷），最小 8 Bytes。
 * - **Checksum**：M1 可写 0（不校验）；完整实现需含 IPv4 伪首部。
 * - **Protocol 号**：在 IPv4 头里为 17（`kUDPProtocolNumber`），不在 UDP 头内。
 *
 * @see RFC 768
 * @see docs/m1.md
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace netstack::header {

/** @brief 无选项时 UDP 头最小长度（字节）。 */
inline constexpr size_t kUDPMinimumSize = 8;

/** @brief IP 协议字段中的 UDP 编号（与 TCP 的 6 并列）。 */
inline constexpr uint8_t kUDPProtocolNumber = 17;

/** @name UDP 头内各字段的字节偏移 */
///@{
inline constexpr size_t kUDPSrcPort = 0;
inline constexpr size_t kUDPDstPort = 2;
inline constexpr size_t kUDPLength = 4;
inline constexpr size_t kUDPChecksum = 6;
///@}

/**
 * @brief 编码 UDP 头时使用的逻辑字段（对标 UDPFields）。
 *
 * length 应等于 8 + payload 字节数；M1 echo 测试里 checksum 常为 0。
 */
struct UDPFields {
  uint16_t src_port{};
  uint16_t dst_port{};
  uint16_t length{};
  uint16_t checksum{};
};

/**
 * @brief 非拥有的 UDP 头视图（类似 Go `type UDP []byte`）。
 *
 * 调用方持有 `std::vector` 或包内缓冲区；本类仅通过 span 读写。
 * 入站时 `pkt.Data()` 在 IPv4 剥头后以 UDP 头开始。
 */
class UDPHeader {
 public:
  explicit UDPHeader(std::span<uint8_t> data) : data_(data) {}

  uint16_t SourcePort() const;
  uint16_t DestinationPort() const;
  uint16_t Length() const;
  uint16_t ChecksumField() const;

  void SetSourcePort(uint16_t port);
  void SetDestinationPort(uint16_t port);
  void SetLength(uint16_t length);
  void SetChecksumField(uint16_t checksum);

  void Encode(const UDPFields& fields);

  /**
   * @brief 从 transport 头前 8 字节解析源/目的端口（demuxer 用）。
   * @return 缓冲区不足 8 字节时 false。
   */
  static bool ParsePorts(std::span<const uint8_t> data, uint16_t& src,
                         uint16_t& dst);

  /**
   * @brief 检查 Length 字段是否与 buffer 一致。
   * @param udp_size 从 UDP 头起到缓冲区末尾的字节数（通常等于 data_.size()）。
   */
  bool IsValid(size_t udp_size) const;

 private:
  std::span<uint8_t> data_;
};

}  // namespace netstack::header
