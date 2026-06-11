/**
 * @file tcp.hh
 * @brief TCP 报文头视图（M2 无选项，固定 20 字节）。
 *
 * @see RFC 793
 * @see references/tcpip/header/tcp.go
 * @see docs/m2.md
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace netstack::header {

inline constexpr size_t kTCPMinimumSize = 20;
inline constexpr uint8_t kTCPProtocolNumber = 6;

inline constexpr size_t kTCPSrcPort = 0;
inline constexpr size_t kTCPDstPort = 2;
inline constexpr size_t kTCPSeqNum = 4;
inline constexpr size_t kTCPAckNum = 8;
inline constexpr size_t kTCPDataOffset = 12;
inline constexpr size_t kTCPFlagsOffset = 13;
inline constexpr size_t kTCPWindowSize = 14;
inline constexpr size_t kTCPChecksum = 16;

enum class TCPFlags : uint8_t {
  kFin = 0x01,
  kSyn = 0x02,
  kRst = 0x04,
  kPsh = 0x08,
  kAck = 0x10,
};

constexpr TCPFlags operator|(TCPFlags a, TCPFlags b) {
  return static_cast<TCPFlags>(static_cast<uint8_t>(a) |
                               static_cast<uint8_t>(b));
}

constexpr bool HasFlag(uint8_t flags, TCPFlags f) {
  return (flags & static_cast<uint8_t>(f)) != 0;
}

struct TCPFields {
  uint16_t src_port{};
  uint16_t dst_port{};
  uint32_t seq_num{};
  uint32_t ack_num{};
  uint8_t data_offset = kTCPMinimumSize;
  uint8_t flags{};
  uint16_t window_size = 65535;
  uint16_t checksum{};
};

class TCPHeader {
 public:
  explicit TCPHeader(std::span<uint8_t> data) : data_(data) {}

  uint16_t SourcePort() const;
  uint16_t DestinationPort() const;
  uint32_t SequenceNumber() const;
  uint32_t AcknowledgmentNumber() const;
  uint8_t DataOffset() const;
  uint8_t Flags() const;
  uint16_t WindowSize() const;

  void Encode(const TCPFields& fields);

  static bool ParsePorts(std::span<const uint8_t> data, uint16_t& src,
                         uint16_t& dst);

  bool IsValid(size_t tcp_size) const;

  size_t HeaderLength() const { return DataOffset(); }

  std::span<const uint8_t> Payload() const;

 private:
  std::span<uint8_t> data_;
};

}  // namespace netstack::header
