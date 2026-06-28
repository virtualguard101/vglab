/**
 * @file udp.cc
 * @brief UDP 头编解码实现。
 *
 * 端口与 Length 均为 **大端（网络序）**，与 IPv4 头一致。
 *
 * ## 固定头布局（8 字节）
 *
 * Src Port(2) | Dst Port(2) | Length(2) | Checksum(2) | Payload...
 *
 * Length 含 8 字节头本身；`IsValid` 防止 Length 大于实际缓冲区（截断/伪造）。
 *
 * ## 校验和（教学栈 M1）
 *
 * IPv4 下 UDP 校验和可选（0 表示未计算）；本模块只编解码字段，不算伪首部。
 * 发送路径若需校验和，由 `udp::Endpoint` 在封装时填写。
 *
 * @see include/netstack/header/udp.hh
 * @see docs/m1.md
 */

#include "netstack/header/udp.hh"

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

uint16_t UDPHeader::SourcePort() const {
  return ReadBE16(data_.data() + kUDPSrcPort);
}

uint16_t UDPHeader::DestinationPort() const {
  return ReadBE16(data_.data() + kUDPDstPort);
}

/** @brief UDP 报文总长度（头 + 载荷），非仅载荷。*/
uint16_t UDPHeader::Length() const {
  return ReadBE16(data_.data() + kUDPLength);
}

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

/** @brief 按 fields 顺序写入四个 16 位字段。 */
void UDPHeader::Encode(const UDPFields& fields) {
  SetSourcePort(fields.src_port);
  SetDestinationPort(fields.dst_port);
  SetLength(fields.length);
  SetChecksumField(fields.checksum);
}

/**
 * @brief 静态解析：从 L4 头前 8 字节读出源/目的端口（demux 用）。
 *
 * 不要求完整 UDP 报文，仅需 `kUDPMinimumSize` 字节即可。
 */
bool UDPHeader::ParsePorts(std::span<const uint8_t> data, uint16_t& src,
                           uint16_t& dst) {
  if (data.size() < kUDPMinimumSize) {
    return false;
  }
  src = ReadBE16(data.data() + kUDPSrcPort);
  dst = ReadBE16(data.data() + kUDPDstPort);
  return true;
}

/**
 * @brief 防止 Length 声称比实际缓冲区更长（常见伪造/截断攻击面）。
 */
bool UDPHeader::IsValid(size_t udp_size) const {
  if (data_.size() < kUDPMinimumSize || udp_size < kUDPMinimumSize) {
    return false;
  }
  const uint16_t len = Length();
  return len >= kUDPMinimumSize && len <= udp_size;
}

}  // namespace netstack::header
