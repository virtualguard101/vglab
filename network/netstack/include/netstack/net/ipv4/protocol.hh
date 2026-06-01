/**
 * @file protocol.hh
 * @brief IPv4 网络层协议（入站 HandlePacket）。
 */

#pragma once

#include "netstack/stack/network_protocol.hh"

namespace netstack::net::ipv4 {

/** @brief IPv4 网络协议实现（M0：校验 + 剥头 + 交付传输层，无分片）。 */
class Protocol : public stack::NetworkProtocol {
 public:
  stack::NetworkProtocolNumber Number() const override;
  int MinimumPacketSize() const override;
  void ParseAddresses(const std::vector<uint8_t>& packet, stack::Address& src,
                      stack::Address& dst) const override;
  void HandlePacket(stack::Route* route, stack::PacketBuffer pkt) override;

  uint64_t MalformedPacketsReceived() const {
    return malformed_packets_received_;
  }
  uint64_t PacketsDelivered() const { return packets_delivered_; }

 private:
  uint64_t malformed_packets_received_{0};
  uint64_t packets_delivered_{0};
};

}  // namespace netstack::net::ipv4
