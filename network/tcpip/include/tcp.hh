#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

using ByteVector = std::vector<uint8_t>;

struct TCPFlags {
  bool fin;
  bool syn;
  bool rst;
  bool psh;
  bool ack;
  bool urg;
  bool ece;
  bool cwr;
};

struct TCPHeader {
  uint16_t src_port;
  uint16_t dst_port;
  uint32_t seq_num;
  uint32_t ack_num;
  uint8_t data_offset;
  uint8_t reserved;
  TCPFlags flags;
  uint16_t window_size;
  uint16_t checksum;
  uint16_t urgent_pointer;
  ByteVector options;
};

// https://datatracker.ietf.org/doc/html/rfc9293#name-header-format
// 0                   1                   2                   3
// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |          Source Port          |       Destination Port        |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                        Sequence Number                        |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    Acknowledgment Number                      |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  Data |       |C|E|U|A|P|R|S|F|                               |
// | Offset| Rsrvd |W|C|R|C|S|S|Y|I|            Window             |
// |       |       |R|E|G|K|H|T|N|N|                               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           Checksum            |         Urgent Pointer        |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           [Options]                           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                                               :
// :                            payload                            :
// :                                                               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
struct TCPPacket {
  TCPHeader header;
  ByteVector payload;
};

/// @param data Wire-format TCP segment (header + payload).
/// @param segment_len Total segment length in bytes (e.g. from IPv4 payload
/// size).
std::optional<TCPPacket> Decoder(const uint8_t* data, std::size_t segment_len);

/// Decode a TCP segment and verify checksum (requires IPv4 pseudo-header).
/// @param data Wire-format TCP segment (header + payload).
/// @param segment_len Total segment length in bytes (e.g. from IPv4 payload
/// size).
/// @param src_ip Source IPv4 address, 4 bytes in network (wire) order.
/// @param dst_ip Destination IPv4 address, 4 bytes in network (wire) order.
std::optional<TCPPacket> Decoder(const uint8_t* data, std::size_t segment_len,
                                 const uint8_t* src_ip, const uint8_t* dst_ip);

std::optional<ByteVector> Encoder(const TCPPacket& packet);