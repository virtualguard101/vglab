/**
 * @file channel.cc
 * @brief 测试用 channel 链路实现（对标 tcpip/link/channel）。
 *
 * ## 为何需要 channel？
 *
 * - 真实 TUN/网卡需要权限与环境，不利于 CI。
 * - channel 在内存中模拟「收包 / 发包」：
 *   - **InjectInbound**：测试代码直接调用，等价于网卡收到一帧；
 *   - **WritePacket**：协议栈出站时把包放入 `outbound_`，测试用 DrainOutbound
 * 读取。
 *
 * ## 与 loopback 的区别
 *
 * - loopback：WritePacket 立即环回到同一 NIC 的 DeliverNetworkPacket。
 * - channel：入站由测试注入；出站进入队列，**不会**自动环回。
 *
 * @see include/netstack/link/channel.hh
 * @see docs/m1.md（UDP echo 将主要用 channel 断言出站）
 */

#include "netstack/link/channel.hh"

namespace netstack::link {

ChannelEndpoint::ChannelEndpoint(size_t queue_capacity, uint32_t mtu,
                                 LinkAddress link_addr)
    : mtu_(mtu), link_addr_(link_addr), capacity_(queue_capacity) {}

uint32_t ChannelEndpoint::MTU() const { return mtu_; }

stack::LinkEndpointCapabilities ChannelEndpoint::Capabilities() const {
  return 0;
}

/** @brief M0/M1 测试帧无以太网头，与 loopback 一致。*/
uint16_t ChannelEndpoint::MaxHeaderLength() const { return 0; }

LinkAddress ChannelEndpoint::LinkAddr() const { return link_addr_; }

/**
 * @brief 出站：将包存入 outbound_ 队列，供测试 DrainOutbound。
 *
 * 若设置了 capacity_ 且队列已满，返回 kInvalidEndpointState（背压模拟）。
 */
stack::StackResult ChannelEndpoint::WritePacket(
    stack::Route* route, const stack::GSO* gso,
    stack::NetworkProtocolNumber protocol, stack::PacketBuffer pkt) {
  (void)route;
  (void)gso;
  if (capacity_ > 0 && outbound_.size() >= capacity_) {
    return stack::StackError{stack::ErrorCode::kInvalidEndpointState};
  }
  outbound_.push_back(ChannelPacketInfo{std::move(pkt), protocol});
  return std::nullopt;
}

/** @brief NIC 构造时调用；之后 InjectInbound 才能调到 DeliverNetworkPacket。*/
void ChannelEndpoint::Attach(stack::NetworkDispatcher* dispatcher) {
  dispatcher_ = dispatcher;
}

bool ChannelEndpoint::IsAttached() const { return dispatcher_ != nullptr; }

/**
 * @brief 测试注入入站包（默认 remote 链路地址为空）。
 *
 * protocol 通常为 kIPv4ProtocolNumber (0x0800)；pkt.Data() 为**裸 IPv4 报文**。
 */
void ChannelEndpoint::InjectInbound(stack::NetworkProtocolNumber protocol,
                                    stack::PacketBuffer pkt) {
  InjectLinkAddr(protocol, LinkAddress{}, std::move(pkt));
}

/**
 * @brief 带远端链路地址的入站注入（多 NIC / L2 场景预留）。
 */
void ChannelEndpoint::InjectLinkAddr(stack::NetworkProtocolNumber protocol,
                                     LinkAddress remote,
                                     stack::PacketBuffer pkt) {
  if (dispatcher_ == nullptr) {
    return;
  }
  dispatcher_->DeliverNetworkPacket(*this, remote, LinkAddress{}, protocol,
                                    std::move(pkt));
}

/** @brief 取出并清空出站队列（swap 避免拷贝 deque 内所有包）。*/
std::deque<ChannelPacketInfo> ChannelEndpoint::DrainOutbound() {
  std::deque<ChannelPacketInfo> out;
  out.swap(outbound_);
  return out;
}

std::unique_ptr<stack::InjectableLinkEndpoint> NewChannel(
    size_t queue_capacity, uint32_t mtu, LinkAddress link_addr) {
  return std::make_unique<ChannelEndpoint>(queue_capacity, mtu, link_addr);
}

}  // namespace netstack::link
