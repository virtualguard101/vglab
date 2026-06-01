/**
 * @file stack.cc
 * @brief 协议栈核心：网络协议与 NIC 的生命周期管理。
 *
 * ## 职责（对标 references/tcpip/stack/stack.go）
 *
 * - 持有已注册的 **NetworkProtocol**（如 IPv4）；
 * - 创建 **NIC**，并把链路 endpoint 与 NIC 绑定；
 * - 在「先注册协议、后建 NIC」或相反顺序下，保证每个 NIC 都能 demux 到协议。
 *
 * ## 入站数据路径
 *
 * @code
 * LinkEndpoint::WritePacket / Channel::InjectInbound
 *   → NIC::DeliverNetworkPacket (NetworkDispatcher)
 *   → NetworkProtocol::HandlePacket (如 ipv4::Protocol)
 *   → TransportDispatcher::DeliverTransportPacket (M0: Recording stub)
 * @endcode
 *
 * @see include/netstack/stack/stack.hh
 * @see docs/m0.md
 */

#include "netstack/stack/stack.hh"

#include "netstack/header/ipv4.hh"
#include "netstack/net/ipv4/protocol.hh"

namespace netstack::stack {

/**
 * @brief 注册一种网络层协议，并挂到**已存在**的所有 NIC 上。
 *
 * 典型顺序：先 AddNetworkProtocol(ipv4)，再 CreateNIC(loopback)。
 * 若 NIC 已先创建，本函数会遍历 nics_ 补 RegisterProtocol。
 */
void Stack::AddNetworkProtocol(std::unique_ptr<NetworkProtocol> protocol) {
  const auto number = protocol->Number();
  protocols_[number] = std::move(protocol);
  NetworkProtocol* raw = protocols_[number].get();
  for (auto& [id, nic] : nics_) {
    (void)id;
    nic->RegisterProtocol(number, raw);
  }
}

/**
 * @brief 创建 NIC：分配 NICID，Attach 链路层，并注册当前所有网络协议。
 *
 * @return 新 NIC 的 ID（从 1 递增，对标 tcpip.NICID）。
 */
NICID Stack::CreateNIC(std::unique_ptr<LinkEndpoint> link_ep) {
  const NICID id = next_nic_id_++;
  auto nic = std::make_unique<NIC>(id, std::move(link_ep));
  for (const auto& [number, proto] : protocols_) {
    nic->RegisterProtocol(number, proto.get());
  }
  nics_[id] = std::move(nic);
  return id;
}

NIC* Stack::GetNIC(NICID id) {
  auto it = nics_.find(id);
  return it == nics_.end() ? nullptr : it->second.get();
}

const NIC* Stack::GetNIC(NICID id) const {
  auto it = nics_.find(id);
  return it == nics_.end() ? nullptr : it->second.get();
}

NetworkProtocol* Stack::FindProtocol(NetworkProtocolNumber number) {
  auto it = protocols_.find(number);
  return it == protocols_.end() ? nullptr : it->second.get();
}

const NetworkProtocol* Stack::FindProtocol(NetworkProtocolNumber number) const {
  auto it = protocols_.find(number);
  return it == protocols_.end() ? nullptr : it->second.get();
}

/**
 * @brief 聚合指定 NIC 与 IPv4 协议的接收计数，供验收测试使用。
 *
 * IPv4 计数通过 dynamic_cast 获取；未注册 IPv4 时仅填 nic 字段。
 */
StackStats Stack::GetStats(NICID id) const {
  StackStats stats{};
  const NIC* nic = GetNIC(id);
  if (nic == nullptr) {
    return stats;
  }
  stats.nic.rx_packets = nic->RxPackets();
  stats.nic.rx_bytes = nic->RxBytes();
  stats.nic.unknown_protocol_rcvd = nic->UnknownProtocolRcvd();
  stats.nic.malformed_rcvd = nic->MalformedRcvd();

  if (const auto* ipv4 = dynamic_cast<const net::ipv4::Protocol*>(
          FindProtocol(header::kIPv4ProtocolNumber))) {
    stats.ipv4_malformed_received = ipv4->MalformedPacketsReceived();
    stats.ipv4_packets_delivered = ipv4->PacketsDelivered();
  }
  return stats;
}

}  // namespace netstack::stack
