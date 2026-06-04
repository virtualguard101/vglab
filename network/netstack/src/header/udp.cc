/**
 * @file udp.cc
 * @brief UDP 头编解码实现。
 */

#include "netstack/header/udp.hh"

namespace netstack::header {

namespace {

uint16_t ReadBE16(const uint8_t* p) {
  return (static_cast<uint16_t>(p[0]) << 8) | p[1];
}

void WriteBE16(uint8_t* p, uint16_t v) {
  p[0] = static_cast<uint8_t>(v >> 8);
  p[1] = static_cast<uint8_t>(v);
}

}  // namespace

uint16_t UDPHeader::SourcePort() const {
  return ReadBE16(data_.data() + kUDPSrcPort);
}

uint16_t UDPHeader::DestinationPort() const {
  return ReadBE16(data_.data() + kUDPDstPort);
}

uint16_t UDPHeader::Length() const { return ReadBE16(data_.data() + kUDPLength); }

uint16_t UDPHeader::ChecksumField() const {
  return ReadBE16(data_.data() + kUDPChecksum);
}

void UDPHeader::SetSourcePort(uint16_t port) {
  WriteBE16(data_.data() + kUDPSrcPort, port);
}

void UDPHeader::SetDestinationPort(uint16_t port) {
  WriteBE16(data_.data() + kUDPDstPort, port);
}

void UDPHeader::SetLength(uint16_t length) {
  WriteBE16(data_.data() + kUDPLength, length);
}

void UDPHeader::SetChecksumField(uint16_t checksum) {
  WriteBE16(data_.data() + kUDPChecksum, checksum);
}

void UDPHeader::Encode(const UDPFields& fields) {
  SetSourcePort(fields.src_port);
  SetDestinationPort(fields.dst_port);
  SetLength(fields.length);
  SetChecksumField(fields.checksum);
}

bool UDPHeader::ParsePorts(std::span<const uint8_t> data, uint16_t& src,
                           uint16_t& dst) {
  if (data.size() < kUDPMinimumSize) {
    return false;
  }
  src = ReadBE16(data.data() + kUDPSrcPort);
  dst = ReadBE16(data.data() + kUDPDstPort);
  return true;
}

bool UDPHeader::IsValid(size_t udp_size) const {
  if (data_.size() < kUDPMinimumSize || udp_size < kUDPMinimumSize) {
    return false;
  }
  const uint16_t len = Length();
  return len >= kUDPMinimumSize && len <= udp_size;
}

}  // namespace netstack::header
