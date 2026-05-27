#include "tcp.hh"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <vector>

#include "utils.hh"

namespace {

constexpr std::size_t kMinTcpHeaderLen = 20;

bool IsValidDataOffset(std::size_t data_offset, std::size_t segment_len) {
  return data_offset >= kMinTcpHeaderLen && data_offset % 4 == 0 &&
         data_offset <= segment_len;
}

}  // unnamed namespace

std::optional<TCPPacket> Decoder(const uint8_t* data, std::size_t segment_len) {
  if (data == nullptr || segment_len < kMinTcpHeaderLen) {
    return std::nullopt;
  }

  const uint8_t data_offset = static_cast<uint8_t>((data[12] >> 4) * 4);
  if (!IsValidDataOffset(data_offset, segment_len)) {
    std::cerr << "Invalid TCP data offset: " << static_cast<int>(data_offset)
              << '\n';
    return std::nullopt;
  }

  TCPHeader header{};
  const uint8_t fl = data[13];

  // Decode header
  try {
    header = {
        .src_port = static_cast<uint16_t>(ReadBE16(data)),
        .dst_port = static_cast<uint16_t>(ReadBE16(data + 2)),
        .seq_num = static_cast<uint32_t>(ReadBE32(data + 4)),
        .ack_num = static_cast<uint32_t>(ReadBE32(data + 8)),
        .data_offset = static_cast<uint8_t>((data[12] >> 4) * 4),
        .reserved = static_cast<uint8_t>(data[12] & 0x0F),
        .flags =
            {
                // Read flags bit by bit with right-shift and bitwise AND
                .fin = static_cast<bool>((fl >> 0) & 1),
                .syn = static_cast<bool>((fl >> 1) & 1),
                .rst = static_cast<bool>((fl >> 2) & 1),
                .psh = static_cast<bool>((fl >> 3) & 1),
                // Also can use mask read the bit directly
                .ack = (fl & 0x10) != 0,
                .urg = (fl & 0x20) != 0,
                .ece = (fl & 0x40) != 0,
                .cwr = (fl & 0x80) != 0,
            },
        .window_size = ReadBE16(data + 14),
        .checksum = ReadBE16(data + 16),
        .urgent_pointer = ReadBE16(data + 18),
    };
    header.options.assign(data + kMinTcpHeaderLen, data + data_offset);
  } catch (const std::exception& e) {
    std::cerr << "Error decoding TCP header: " << e.what() << '\n';
    return std::nullopt;
  }

  // NOTE: TCP checksum requires IPv4 pseudo-header (src/dst IP + protocol +
  // length). Use the overload Decoder(data, len, src_ip, dst_ip) when you have
  // IP addresses.

  // Assemble packet and return
  try {
    return TCPPacket{
        .header = header,
        .payload = std::vector<uint8_t>(data + data_offset, data + segment_len),
    };
  } catch (const std::exception& e) {
    std::cerr << "Error assembling TCP packet: " << e.what() << '\n';
    return std::nullopt;
  }
}

std::optional<TCPPacket> Decoder(const uint8_t* data, std::size_t segment_len,
                                 const uint8_t* src_ip, const uint8_t* dst_ip) {
  auto pkt = Decoder(data, segment_len);
  if (!pkt) {
    return std::nullopt;
  }

  if (!VerifyTCPChecksum(data, segment_len, src_ip, dst_ip)) {
    std::cerr << "Invalid TCP checksum\n";
    return std::nullopt;
  }
  return pkt;
}

std::optional<ByteVector> Encoder(const TCPPacket& packet) {
  ByteVector payload{};
  if (!packet.payload.empty()) {
    try {
      payload.assign(packet.payload.begin(), packet.payload.end());
    } catch (const std::exception& e) {
      std::cerr << "Error encoding TCP packet payload: " << e.what() << '\n';
      return std::nullopt;
    }
  }

  ByteVector data{};

  auto append_be16 = [&data](uint16_t value) {
    data.push_back(static_cast<uint8_t>(value >> 8));
    data.push_back(static_cast<uint8_t>(value));
  };
  auto append_be32 = [&data](uint32_t value) {
    data.push_back(static_cast<uint8_t>(value >> 24));
    data.push_back(static_cast<uint8_t>(value >> 16));
    data.push_back(static_cast<uint8_t>(value >> 8));
    data.push_back(static_cast<uint8_t>(value));
  };

  try {
    append_be16(packet.header.src_port);
    append_be16(packet.header.dst_port);
    append_be32(packet.header.seq_num);
    append_be32(packet.header.ack_num);

    const uint8_t header_len =
        (packet.header.data_offset != 0)
            ? packet.header.data_offset
            : static_cast<uint8_t>(kMinTcpHeaderLen +
                                   packet.header.options.size());
    // TCP Data Offset is measured in 32-bit words (4 bytes).
    const uint8_t data_offset_words = static_cast<uint8_t>(header_len / 4);
    data.push_back(static_cast<uint8_t>(data_offset_words << 4) |
                   static_cast<uint8_t>(packet.header.reserved & 0x0F));

    uint8_t fl = 0;
    if (packet.header.flags.fin) fl |= 0x01;
    if (packet.header.flags.syn) fl |= 0x02;
    if (packet.header.flags.rst) fl |= 0x04;
    if (packet.header.flags.psh) fl |= 0x08;
    if (packet.header.flags.ack) fl |= 0x10;
    if (packet.header.flags.urg) fl |= 0x20;
    if (packet.header.flags.ece) fl |= 0x40;
    if (packet.header.flags.cwr) fl |= 0x80;
    data.push_back(fl);

    append_be16(packet.header.window_size);
    // checksum is written by caller (requires IPv4 pseudo-header)
    append_be16(packet.header.checksum);
    append_be16(packet.header.urgent_pointer);
    data.insert(data.end(), packet.header.options.begin(),
                packet.header.options.end());
  } catch (const std::exception& e) {
    std::cerr << "Error encoding TCP packet header: " << e.what() << '\n';
    return std::nullopt;
  }

  // Assemble raw packet data and return
  try {
    data.insert(data.end(), payload.begin(), payload.end());
  } catch (const std::exception& e) {
    std::cerr << "Error encoding TCP packet payload: " << e.what() << '\n';
    return std::nullopt;
  }

  return data;
}
