#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct IPv4Address {
  std::array<uint8_t, 4> octets{};

  /// Dotted-decimal user input (e.g. "192.168.1.1").
  static std::optional<IPv4Address> Parse(std::string_view str);

  /// Four octets in network (wire) order from a packet buffer.
  static IPv4Address FromWire(const uint8_t* data);

  /// Write four octets in network order into a packet buffer.
  void WriteWire(uint8_t* data) const;

  std::string ToString() const;
};

struct IPv4Header {
  uint8_t version{4};
  uint8_t header_len;
  uint8_t type_of_service;
  uint16_t total_length;
  uint16_t identification;
  uint8_t flags;
  uint16_t fragment_offset;
  uint8_t time_to_live;
  uint8_t protocol;
  uint16_t header_checksum;
  IPv4Address src_addr;
  IPv4Address dst_addr;
  std::vector<uint8_t> options;
};

/// https://datatracker.ietf.org/doc/html/rfc791#section-3.1
/// 0                   1                   2                   3
/// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |Version|  IHL  |Type of Service|          Total Length         |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |         Identification        |Flags|      Fragment Offset    |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |  Time to Live |    Protocol   |         Header Checksum       |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |                       Source Address                          |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |                    Destination Address                        |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// |                    Options                    |    Padding    |
/// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
struct IPv4Packet {
  IPv4Header header;
  std::vector<uint8_t> payload;
};

/// @brief Decode an IPv4 header from a packet buffer.
/// @param data Pointer to the packet buffer.
/// @return Optional IPv4 packet if the header is valid, otherwise nullopt.
std::optional<IPv4Packet> Decoder(const uint8_t* data);

/// @brief Encode an IPv4 packet into a buffer.
/// @param packet The IPv4 packet to encode.
/// @param data Pointer to the buffer to encode the packet into.
/// @return The number of bytes encoded, or nullopt if the encoding fails.
std::optional<uint8_t> Encoder(const IPv4Packet& packet, uint8_t* data);
