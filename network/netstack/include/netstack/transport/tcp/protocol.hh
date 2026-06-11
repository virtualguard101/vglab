/**
 * @file protocol.hh
 * @brief TCP 传输层协议（M2）。
 */

#pragma once

#include "netstack/stack/transport.hh"

namespace netstack::transport::tcp {

class Protocol : public stack::TransportProtocol {
 public:
  stack::TransportProtocolNumber Number() const override;
  int MinimumPacketSize() const override;
  bool ParsePorts(std::span<const uint8_t> transport_hdr, uint16_t& src,
                  uint16_t& dst) const override;
};

}  // namespace netstack::transport::tcp
