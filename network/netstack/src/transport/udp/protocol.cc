/**
 * @file protocol.cc
 * @brief UDP 作为 TransportProtocol 的元数据实现。
 *
 * TransportDemuxer 通过本类获取：
 * - 协议号 17（kUDPProtocolNumber）
 * - 最小 UDP 头长 8 字节
 * - ParsePorts：从 L4 头提取源/目的端口
 *
 * 实际 echo 逻辑在 `udp::Endpoint::HandlePacket`。
 *
 * @see include/netstack/transport/udp/protocol.hh
 * @see docs/m1.md
 */

#include "netstack/transport/udp/protocol.hh"

#include "netstack/header/udp.hh"

namespace netstack::transport::udp {

stack::TransportProtocolNumber Protocol::Number() const {
  return header::kUDPProtocolNumber;
}

int Protocol::MinimumPacketSize() const {
  return static_cast<int>(header::kUDPMinimumSize);
}

bool Protocol::ParsePorts(std::span<const uint8_t> transport_hdr, uint16_t& src,
                          uint16_t& dst) const {
  return header::UDPHeader::ParsePorts(transport_hdr, src, dst);
}

}  // namespace netstack::transport::udp
