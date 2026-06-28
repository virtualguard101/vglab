/**
 * @file protocol.cc
 * @brief TCP 作为 TransportProtocol 的元数据实现（M2）。
 *
 * TransportDemuxer 入站分发前调用：
 * - `Number()` → 6
 * - `MinimumPacketSize()` → 20（无选项 TCP 头）
 * - `ParsePorts()` → 源/目的端口，与 Route 中 IP 组成四元组
 *
 * 连接状态机不在此类，而在 `tcp::Connection` / `tcp::Listener`。
 *
 * @see include/netstack/transport/tcp/protocol.hh
 * @see docs/m2.md
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
