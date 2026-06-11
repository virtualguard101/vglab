/**
 * @file tcp.cc
 */

#include "netstack/header/tcp.hh"

namespace netstack::header {

namespace {

uint16_t ReadBE16(const uint8_t* p) {
  return (static_cast<uint16_t>(p[0]) << 8) | p[1];
}

uint32_t ReadBE32(const uint8_t* p) {
  return (static_cast<uint32_t>(p[0]) << 24) |
         (static_cast<uint32_t>(p[1]) << 16) |
         (static_cast<uint32_t>(p[2]) << 8) | p[3];
}

void WriteBE16(uint8_t* p, uint16_t v) {
  p[0] = static_cast<uint8_t>(v >> 8);
  p[1] = static_cast<uint8_t>(v);
}

void WriteBE32(uint8_t* p, uint32_t v) {
  p[0] = static_cast<uint8_t>(v >> 24);
  p[1] = static_cast<uint8_t>(v >> 16);
  p[2] = static_cast<uint8_t>(v >> 8);
  p[3] = static_cast<uint8_t>(v);
}

}  // namespace

uint16_t TCPHeader::SourcePort() const {
  return ReadBE16(data_.data() + kTCPSrcPort);
}

uint16_t TCPHeader::DestinationPort() const {
  return ReadBE16(data_.data() + kTCPDstPort);
}

uint32_t TCPHeader::SequenceNumber() const {
  return ReadBE32(data_.data() + kTCPSeqNum);
}

uint32_t TCPHeader::AcknowledgmentNumber() const {
  return ReadBE32(data_.data() + kTCPAckNum);
}

uint8_t TCPHeader::DataOffset() const {
  return static_cast<uint8_t>((data_[kTCPDataOffset] >> 4) * 4);
}

uint8_t TCPHeader::Flags() const { return data_[kTCPFlagsOffset]; }

uint16_t TCPHeader::WindowSize() const {
  return ReadBE16(data_.data() + kTCPWindowSize);
}

void TCPHeader::Encode(const TCPFields& fields) {
  WriteBE16(data_.data() + kTCPSrcPort, fields.src_port);
  WriteBE16(data_.data() + kTCPDstPort, fields.dst_port);
  WriteBE32(data_.data() + kTCPSeqNum, fields.seq_num);
  WriteBE32(data_.data() + kTCPAckNum, fields.ack_num);
  const uint8_t offset_words =
      static_cast<uint8_t>((fields.data_offset / 4) & 0x0f);
  data_[kTCPDataOffset] = static_cast<uint8_t>(offset_words << 4);
  data_[kTCPFlagsOffset] = fields.flags;
  WriteBE16(data_.data() + kTCPWindowSize, fields.window_size);
  WriteBE16(data_.data() + kTCPChecksum, fields.checksum);
}

bool TCPHeader::ParsePorts(std::span<const uint8_t> data, uint16_t& src,
                           uint16_t& dst) {
  if (data.size() < kTCPMinimumSize) {
    return false;
  }
  src = ReadBE16(data.data() + kTCPSrcPort);
  dst = ReadBE16(data.data() + kTCPDstPort);
  return true;
}

bool TCPHeader::IsValid(size_t tcp_size) const {
  if (data_.size() < kTCPMinimumSize || tcp_size < kTCPMinimumSize) {
    return false;
  }
  const auto hlen = DataOffset();
  return hlen >= kTCPMinimumSize && hlen <= tcp_size;
}

std::span<const uint8_t> TCPHeader::Payload() const {
  const auto hlen = DataOffset();
  if (data_.size() <= hlen) {
    return {};
  }
  return std::span<const uint8_t>(data_.data() + hlen, data_.size() - hlen);
}

}  // namespace netstack::header
