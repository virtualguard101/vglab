/**
 * @file udp.hh
 * @brief UDP 报文头视图与编解码（对标 references/tcpip/header/udp.go）。
 *
 * @see RFC 768
 * @see docs/m1.md
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace netstack::header {

inline constexpr size_t kUDPMinimumSize = 8;
inline constexpr uint8_t kUDPProtocolNumber = 17;

inline constexpr size_t kUDPSrcPort = 0;
inline constexpr size_t kUDPDstPort = 2;
inline constexpr size_t kUDPLength = 4;
inline constexpr size_t kUDPChecksum = 6;

struct UDPFields {
  uint16_t src_port{};
  uint16_t dst_port{};
  uint16_t length{};
  uint16_t checksum{};
};

/**
 * @brief 非拥有的 UDP 头视图（类似 Go `type UDP []byte`）。
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

  /** @brief 从 transport 头前 8 字节解析源/目的端口。 */
  static bool ParsePorts(std::span<const uint8_t> data, uint16_t& src,
                         uint16_t& dst);

  /**
   * @brief Length 字段合法且不超过 buffer 中 UDP 段长度。
   * @param udp_size 从 UDP 头起到 buffer 末尾的长度。
   */
  bool IsValid(size_t udp_size) const;

 private:
  std::span<uint8_t> data_;
};

}  // namespace netstack::header
