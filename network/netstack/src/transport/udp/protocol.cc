/**
 * @file protocol.cc
 * @brief UDP 作为 TransportProtocol 的元数据实现。
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
