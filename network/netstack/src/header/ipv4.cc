/**
 * @file ipv4.cc
 * @brief IPv4Header 编解码与校验（header 模块）。
 *
 * 多字节字段均按 **网络字节序（大端）** 读写，与 RFC 791 及 Wireshark
 * 显示一致。
 */

#include "netstack/header/ipv4.hh"

#include "netstack/header/checksum.hh"

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

int IPv4Header::IPVersion(std::span<const uint8_t> data) {
  if (data.size() < kVersIHL + 1) {
    return -1;
  }
  return static_cast<int>(data[kVersIHL] >> 4);
}

uint8_t IPv4Header::HeaderLength() const {
  return (data_[kVersIHL] & 0x0f) * 4;
}

uint16_t IPv4Header::ID() const { return ReadBE16(data_.data() + kID); }

uint8_t IPv4Header::Protocol() const { return data_[kProtocol]; }

uint8_t IPv4Header::Flags() const {
  return static_cast<uint8_t>(ReadBE16(data_.data() + kFlagsFO) >> 13);
}

uint8_t IPv4Header::TTL() const { return data_[kTTL]; }

uint16_t IPv4Header::FragmentOffset() const {
  return static_cast<uint16_t>(ReadBE16(data_.data() + kFlagsFO) & 0x1fff) << 3;
}

uint16_t IPv4Header::TotalLength() const {
  return ReadBE16(data_.data() + kTotalLengthOffset);
}

uint16_t IPv4Header::ChecksumField() const {
  return ReadBE16(data_.data() + kChecksumOffset);
}

IPv4Address IPv4Header::SourceAddress() const {
  return IPv4Address::FromWire(data_.data() + kSrcAddr);
}

IPv4Address IPv4Header::DestinationAddress() const {
  return IPv4Address::FromWire(data_.data() + kDstAddr);
}

uint16_t IPv4Header::PayloadLength() const {
  return TotalLength() - HeaderLength();
}

std::span<uint8_t> IPv4Header::Payload() {
  const auto hlen = HeaderLength();
  if (data_.size() <= hlen) {
    return {};
  }
  return data_.subspan(hlen);
}

void IPv4Header::SetTotalLength(uint16_t total_length) {
  WriteBE16(data_.data() + kTotalLengthOffset, total_length);
}

void IPv4Header::SetChecksumField(uint16_t checksum) {
  WriteBE16(data_.data() + kChecksumOffset, checksum);
}

void IPv4Header::SetFlagsFragmentOffset(uint8_t flags,
                                        uint16_t fragment_offset) {
  // flags 占高 3 位；fragment_offset 以 8 字节为单位存在低 13 位
  const uint16_t v =
      (static_cast<uint16_t>(flags) << 13) | (fragment_offset >> 3);
  WriteBE16(data_.data() + kFlagsFO, v);
}

void IPv4Header::SetID(uint16_t id) { WriteBE16(data_.data() + kID, id); }

void IPv4Header::SetSourceAddress(IPv4Address addr) {
  addr.WriteWire(data_.data() + kSrcAddr);
}

void IPv4Header::SetDestinationAddress(IPv4Address addr) {
  addr.WriteWire(data_.data() + kDstAddr);
}

uint16_t IPv4Header::CalculateChecksum() const {
  const auto hlen = HeaderLength();
  return Checksum(data_.first(hlen), 0);
}

bool IPv4Header::IsChecksumValid() const {
  // 线上存的是反码；对整头求和应折叠为全 1
  return CalculateChecksum() == 0xffff;
}

bool IPv4Header::IsValid(size_t packet_size) const {
  if (data_.size() < kIPv4MinimumSize) {
    return false;
  }

  const int hlen = HeaderLength();
  const int tlen = TotalLength();
  if (hlen < static_cast<int>(kIPv4MinimumSize) || hlen > tlen ||
      tlen > static_cast<int>(packet_size)) {
    return false;
  }

  if (IPVersion(data_) != kIPv4Version) {
    return false;
  }

  return true;
}

void IPv4Header::Encode(const IPv4Fields& fields) {
  data_[kVersIHL] =
      static_cast<uint8_t>((kIPv4Version << 4) | ((fields.ihl / 4) & 0x0f));
  data_[kTOS] = fields.tos;
  SetTotalLength(fields.total_length);
  SetID(fields.id);
  SetFlagsFragmentOffset(fields.flags, fields.fragment_offset);
  data_[kTTL] = fields.ttl;
  data_[kProtocol] = fields.protocol;
  SetChecksumField(fields.checksum);
  SetSourceAddress(fields.src_addr);
  SetDestinationAddress(fields.dst_addr);
}

void IPv4Header::EncodePartial(uint16_t partial_checksum,
                               uint16_t total_length) {
  SetTotalLength(total_length);
  const auto folded =
      Checksum(data_.subspan(kTotalLengthOffset, 2), partial_checksum);
  SetChecksumField(static_cast<uint16_t>(~folded));
}

bool IsV4MulticastAddress(IPv4Address addr) {
  return (addr.octets[0] & 0xf0) == 0xe0;
}

}  // namespace netstack::header
