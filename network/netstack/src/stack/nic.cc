/**
 * @file nic.cc
 * @brief 网络接口卡：链路到网络层的入站分发（NetworkDispatcher）。
 *
 * 对标 references/tcpip/stack/nic.go 的 `DeliverNetworkPacket`（约 734 行起）：
 *
 * 1. 按 **NetworkProtocolNumber**（如 0x0800=IPv4）查找已注册协议；
 * 2. 检查包长是否 ≥ 该协议的 MinimumPacketSize()；
 * 3. 调用 `NetworkProtocol::HandlePacket`。
 *
 * 本实现将「未知协议 / 过短包」分别计入统计，便于 M0 验收测试断言。
 *
 * @see include/netstack/stack/nic.hh
 * @see docs/m0.md
 */

#include "netstack/stack/nic.hh"

namespace netstack::stack {

/**
 * @brief 构造 NIC 并 Attach 链路 endpoint。
 *
 * Attach 后 link_ep 在入站/环回时可回调 this->DeliverNetworkPacket。
 */
NIC::NIC(NICID id, std::unique_ptr<LinkEndpoint> link_ep)
    : id_(id), link_ep_(std::move(link_ep)) {
  link_ep_->Attach(this);
}

void NIC::RegisterProtocol(NetworkProtocolNumber number,
                           NetworkProtocol* protocol) {
  protocols_[number] = protocol;
}

/**
 * @brief 链路层入站入口：根据 protocol 号交付网络层。
 *
 * @param link_ep 收包链路（loopback/channel 等）。
 * @param remote 对端链路地址；测试常为空。
 * @param local 本端链路地址；为空时用 link_ep.LinkAddr()。
 * @param protocol 网络协议号（IPv4 为 0x0800）。
 * @param pkt 载荷；对 loopback/channel 的 M0 测试通常为**完整 IPv4 报文**。
 */
void NIC::DeliverNetworkPacket(LinkEndpoint& link_ep, LinkAddress remote,
                               LinkAddress local,
                               NetworkProtocolNumber protocol,
                               PacketBuffer pkt) {
  (void)remote;
  ++rx_packets_;
  rx_bytes_ += pkt.Data().size();

  if (local.octets == LinkAddress{}.octets) {
    local = link_ep.LinkAddr();
  }

  auto it = protocols_.find(protocol);
  if (it == protocols_.end() || it->second == nullptr) {
    // 例如注入 0x0806(ARP) 而栈只注册了 IPv4
    ++unknown_protocol_rcvd_;
    return;
  }

  NetworkProtocol* net_proto = it->second;
  if (pkt.Data().size() < static_cast<size_t>(net_proto->MinimumPacketSize())) {
    // 在进 IPv4 解析前丢弃（如 10 字节短包）
    ++malformed_rcvd_;
    return;
  }

  Route route{};
  route.net_proto = protocol;
  net_proto->HandlePacket(&route, std::move(pkt));
}

}  // namespace netstack::stack
