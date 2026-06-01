/**
 * @file ipv4.cc
 * @brief IPv4Header 编解码与校验（header 模块）。
 *
 * 多字节字段均按 **网络字节序（大端）** 读写，与 RFC 791 及 Wireshark 一致。
 *
 * ## 读路径 vs 写路径
 *
 * - **读**：`SourceAddress()` 等从 `data_` span 解析，不分配堆内存。
 * - **写**：`Encode()` / `SetTotalLength()` 等修改底层 vector（由调用方持有）。
 *
 * @see include/netstack/header/ipv4.hh
 */

#include "netstack/header/ipv4.hh"

#include "netstack/header/checksum.hh"

namespace netstack::header {

namespace {

/** @brief 从缓冲区读取大端 uint16（网络序）。*/
uint16_t ReadBE16(const uint8_t* p) {
  return (static_cast<uint16_t>(p[0]) << 8) | p[1];
}

/** @brief 向缓冲区写入大端 uint16。*/
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

/** @brief IHL 低 4 位 × 4 = 头长度（字节）。*/
uint8_t IPv4Header::HeaderLength() const {
  return (data_[kVersIHL] & 0x0f) * 4;
}

uint16_t IPv4Header::ID() const { return ReadBE16(data_.data() + kID); }

uint8_t IPv4Header::Protocol() const { return data_[kProtocol]; }

uint8_t IPv4Header::Flags() const {
  return static_cast<uint8_t>(ReadBE16(data_.data() + kFlagsFO) >> 13);
}

uint8_t IPv4Header::TTL() const { return data_[kTTL]; }

/** @brief RFC 791：线上 13 位单位为 8 字节，此处还原为字节偏移。*/
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

/**
 * @brief 对当前头（校验和字段应为 0）求 RFC 1071 和，**未取反**。
 */
uint16_t IPv4Header::CalculateChecksum() const {
  const auto hlen = HeaderLength();
  return Checksum(data_.first(hlen), 0);
}

/**
 * @brief 接收校验：对含 checksum 字段的整头求和，折叠后须为 0xFFFF。
 */
bool IPv4Header::IsChecksumValid() const {
  return CalculateChecksum() == 0xffff;
}

/**
 * @brief 结构合法性（不查 checksum）：版本 4、IHL、TotalLength 与 buffer 关系。
 */
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

/**
 * @brief 仅更新 Total Length 与 Checksum（批量相似包时避免重算整头）。
 */
void IPv4Header::EncodePartial(uint16_t partial_checksum,
                               uint16_t total_length) {
  SetTotalLength(total_length);
  const auto folded =
      Checksum(data_.subspan(kTotalLengthOffset, 2), partial_checksum);
  SetChecksumField(static_cast<uint16_t>(~folded));
}

/** @brief 224.0.0.0/4：首字节高 4 位为 1110。*/
bool IsV4MulticastAddress(IPv4Address addr) {
  return (addr.octets[0] & 0xf0) == 0xe0;
}

}  // namespace netstack::header
