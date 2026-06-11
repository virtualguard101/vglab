/**
 * @file protocol.cc
 * @brief TCP 作为 TransportProtocol 的元数据实现（M2）。
 *
 * 与 UDP `protocol.cc` 对称：把 header 层的常量与 ParsePorts 桥接到 stack。
 *
 * @see include/netstack/transport/tcp/protocol.hh
 */

#include "netstack/transport/tcp/protocol.hh"

#include "netstack/header/tcp.hh"

namespace netstack::transport::tcp {

stack::TransportProtocolNumber Protocol::Number() const {
  return header::kTCPProtocolNumber;
}

int Protocol::MinimumPacketSize() const {
  return static_cast<int>(header::kTCPMinimumSize);
}

bool Protocol::ParsePorts(std::span<const uint8_t> transport_hdr, uint16_t& src,
                          uint16_t& dst) const {
  return header::TCPHeader::ParsePorts(transport_hdr, src, dst);
}

}  // namespace netstack::transport::tcp
