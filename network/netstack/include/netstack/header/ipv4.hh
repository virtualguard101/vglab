#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "netstack/address.hh"

namespace netstack::header {

/// --- Field byte offsets in the IPv4 header (references/header/ipv4.go) ---

inline constexpr size_t kVersIHL = 0;  // Version and Internet Header Length
inline constexpr size_t kTOS = 1;      // Type of Service
inline constexpr size_t kTotalLengthOffset = 2;  // Total Length
inline constexpr size_t kID = 4;                 // Identification
inline constexpr size_t kFlagsFO = 6;            // Flags and Fragment Offset
inline constexpr size_t kTTL = 8;                // Time to Live
inline constexpr size_t kProtocol = 9;           // Protocol
inline constexpr size_t kChecksumOffset = 10;    // Checksum
inline constexpr size_t kSrcAddr = 12;           // Source Address
inline constexpr size_t kDstAddr = 16;           // Destination Address

/// --- Sizes and protocol constants ---

inline constexpr size_t kIPv4MinimumSize = 20;
inline constexpr size_t kIPv4MaximumHeaderSize = 60;
inline constexpr size_t kMinIPFragmentPayloadSize = 8;
inline constexpr size_t kIPv4AddressSize = 4;
inline constexpr uint16_t kIPv4ProtocolNumber =
    0x0800;  // EtherType / network proto
inline constexpr uint8_t kIPv4Version = 4;
inline constexpr size_t kIPv4MinimumProcessableDatagramSize = 576;

enum class IPv4Flags : uint8_t {
  MoreFragments = 0x01,
  DontFragment = 0x02,
};

constexpr IPv4Flags operator|(IPv4Flags a, IPv4Flags b) {
  return static_cast<IPv4Flags>(static_cast<uint8_t>(a) |
                                static_cast<uint8_t>(b));
}

constexpr IPv4Flags operator&(IPv4Flags a, IPv4Flags b) {
  return static_cast<IPv4Flags>(static_cast<uint8_t>(a) &
                                static_cast<uint8_t>(b));
}

/// Logical fields for Encode (references/header.IPv4Fields).
struct IPv4Fields {
  uint8_t ihl = kIPv4MinimumSize;  // Internet Header Length
  uint8_t tos = 0;                 // Type of Service
  uint16_t total_length = 0;       // Total Length
  uint16_t id = 0;                 // Identification
  uint8_t flags = 0;               // Flags
  uint16_t fragment_offset = 0;    // Fragment Offset
  uint8_t ttl = 0;                 // Time to Live
  uint8_t protocol = 0;            // Protocol
  uint16_t checksum = 0;           // Checksum
  IPv4Address src_addr{};          // Source Address
  IPv4Address dst_addr{};          // Destination Address
};

/// Non-owning view of an IPv4 header in a packet buffer
/// (references/header.IPv4).
class IPv4Header {
 public:
  explicit IPv4Header(std::span<uint8_t> data) : data_(data) {}

  static int IPVersion(std::span<const uint8_t> data);

  size_t size() const { return data_.size(); }

  uint8_t HeaderLength() const;
  uint16_t ID() const;
  uint8_t Protocol() const;
  uint8_t Flags() const;
  uint8_t TTL() const;
  uint16_t FragmentOffset() const;
  uint16_t TotalLength() const;
  uint16_t ChecksumField() const;

  IPv4Address SourceAddress() const;
  IPv4Address DestinationAddress() const;

  uint16_t PayloadLength() const;
  std::span<uint8_t> Payload();

  void SetTotalLength(uint16_t total_length);
  void SetChecksumField(uint16_t checksum);
  void SetFlagsFragmentOffset(uint8_t flags, uint16_t fragment_offset);
  void SetID(uint16_t id);
  void SetSourceAddress(IPv4Address addr);
  void SetDestinationAddress(IPv4Address addr);

  uint16_t CalculateChecksum() const;
  bool IsChecksumValid() const;

  /// Structural checks only (references/header.IPv4.IsValid).
  bool IsValid(size_t packet_size) const;

  void Encode(const IPv4Fields& fields);
  void EncodePartial(uint16_t partial_checksum, uint16_t total_length);

 private:
  std::span<uint8_t> data_;
};

bool IsV4MulticastAddress(IPv4Address addr);

}  // namespace netstack::header
