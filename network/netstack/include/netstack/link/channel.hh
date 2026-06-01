/**
 * @file channel.hh
 * @brief 测试用 channel link（对标 tcpip/link/channel）。
 *
 * Channel 是**内存中的假网卡**：
 * - 测试通过 InjectInbound 模拟「收到一个 IPv4 帧」；
 * - 协议栈通过 WritePacket 发出的包进入 outbound_ 队列；
 * - 测试调用 DrainOutbound() 断言 echo 回复等（M1 重点）。
 *
 * @see references/tcpip/link/channel
 * @see docs/m1.md
 */

#pragma once

#include <deque>
#include <memory>

#include "netstack/stack/registration.hh"

namespace netstack::link {

/** @brief 出站队列中的一项：包体 + 写入时的网络协议号。 */
struct ChannelPacketInfo {
  stack::PacketBuffer pkt{};
  stack::NetworkProtocolNumber proto{};
};

/**
 * @brief 内存 channel：可注入入站包，出站包进入队列供测试读取。
 *
 * 实现 InjectableLinkEndpoint，故支持 InjectInbound。
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

  /** @brief 取出并清空出站队列（测试断言 echo 时使用）。 */
  std::deque<ChannelPacketInfo> DrainOutbound();

  size_t OutboundCount() const { return outbound_.size(); }

 private:
  stack::NetworkDispatcher* dispatcher_{nullptr};
  uint32_t mtu_{};
  LinkAddress link_addr_{};
  size_t capacity_{};
  std::deque<ChannelPacketInfo> outbound_{};
};

/** @brief 工厂函数：创建可注入的 channel endpoint。 */
std::unique_ptr<stack::InjectableLinkEndpoint> NewChannel(
    size_t queue_capacity, uint32_t mtu, LinkAddress link_addr);

}  // namespace netstack::link
