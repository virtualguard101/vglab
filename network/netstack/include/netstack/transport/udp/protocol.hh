/**
 * @file protocol.hh
 * @brief UDP 传输层协议注册对象（M1）。
 *
 * 实现 `TransportProtocol`，供 demuxer 解析端口与检查最小长度。
 * 不持有 socket 状态；每个 `Endpoint` 单独 Bind。
 *
 * @see references/tcpip/transport/udp/protocol.go
 */

#pragma once

#include "netstack/stack/transport.hh"

namespace netstack::transport::udp {

class Protocol : public stack::TransportProtocol {
 public:
  stack::TransportProtocolNumber Number() const override;
  int MinimumPacketSize() const override;
  bool ParsePorts(std::span<const uint8_t> transport_hdr, uint16_t& src,
                  uint16_t& dst) const override;
};

}  // namespace netstack::transport::udp
