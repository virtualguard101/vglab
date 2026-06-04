/**
 * @file protocol.hh
 * @brief IPv4 网络层协议（入站 HandlePacket）。
 *
 * 实现 stack::NetworkProtocol，负责 M0 入站路径：
 * 校验 IPv4 头 → 剥头 → DeliverTransportPacket。
 *
 * 出站由 `net::ipv4::SendPacket` 完成（M1）；入站会填写 Route 的源/目的 IP。
 *
 * @see src/net/ipv4/protocol.cc
 * @see references/tcpip/network/ipv4/ipv4.go
 */

#pragma once

#include "netstack/stack/network_protocol.hh"

namespace netstack::net::ipv4 {

/**
 * @brief IPv4 网络协议（M0：校验 + 剥头 + 交付传输层，无分片重组）。
 */
class Protocol : public stack::NetworkProtocol {
 public:
  stack::NetworkProtocolNumber Number() const override;
  int MinimumPacketSize() const override;
  void ParseAddresses(const std::vector<uint8_t>& packet, stack::Address& src,
                      stack::Address& dst) const override;
  void HandlePacket(stack::Route* route, stack::PacketBuffer pkt) override;

  /** @brief 因校验/分片等丢弃的入站包计数。 */
  uint64_t MalformedPacketsReceived() const {
    return malformed_packets_received_;
  }
  /** @brief 成功交给 TransportDispatcher 的包数。 */
  uint64_t PacketsDelivered() const { return packets_delivered_; }

 private:
  uint64_t malformed_packets_received_{0};
  uint64_t packets_delivered_{0};
};

}  // namespace netstack::net::ipv4
