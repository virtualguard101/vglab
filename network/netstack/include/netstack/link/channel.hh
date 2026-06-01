/**
 * @file channel.hh
 * @brief 测试用 channel link（对标 tcpip/link/channel）。
 */

#pragma once

#include <deque>
#include <memory>

#include "netstack/stack/registration.hh"

namespace netstack::link {

/** @brief 出站包记录（对标 channel.PacketInfo）。*/
struct ChannelPacketInfo {
  stack::PacketBuffer pkt{};
  stack::NetworkProtocolNumber proto{};
};

/**
 * @brief 内存 channel：可注入入站包，出站包进入队列供测试读取。
 */
class ChannelEndpoint : public stack::InjectableLinkEndpoint {
 public:
  ChannelEndpoint(size_t queue_capacity, uint32_t mtu, LinkAddress link_addr);

  uint32_t MTU() const override;
  stack::LinkEndpointCapabilities Capabilities() const override;
  uint16_t MaxHeaderLength() const override;
  LinkAddress LinkAddr() const override;

  stack::StackResult WritePacket(stack::Route* route, const stack::GSO* gso,
                                 stack::NetworkProtocolNumber protocol,
                                 stack::PacketBuffer pkt) override;

  void Attach(stack::NetworkDispatcher* dispatcher) override;
  bool IsAttached() const override;

  void InjectInbound(stack::NetworkProtocolNumber protocol,
                     stack::PacketBuffer pkt) override;
  void InjectLinkAddr(stack::NetworkProtocolNumber protocol, LinkAddress remote,
                      stack::PacketBuffer pkt);

  /** @brief 取出并清空出站队列。 */
  std::deque<ChannelPacketInfo> DrainOutbound();

  size_t OutboundCount() const { return outbound_.size(); }

 private:
  stack::NetworkDispatcher* dispatcher_{nullptr};
  uint32_t mtu_{};
  LinkAddress link_addr_{};
  size_t capacity_{};
  std::deque<ChannelPacketInfo> outbound_{};
};

std::unique_ptr<stack::InjectableLinkEndpoint> NewChannel(
    size_t queue_capacity, uint32_t mtu, LinkAddress link_addr);

}  // namespace netstack::link
