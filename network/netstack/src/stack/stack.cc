/**
 * @file stack.cc
 * @brief 协议栈核心实现：NIC、协议注册、demuxer 与端口管理。
 *
 * ## 组装顺序（demo / 测试通用）
 *
 * @code
 * Stack s;
 * s.AddNetworkProtocol(ipv4::Protocol);
 * s.AddTransportProtocol(tcp::Protocol);  // 或 udp
 * nic_id = s.CreateNIC(link_endpoint);
 * s.AddAddress(nic_id, local_ipv4);
 * // 应用：Listener::Listen / udp::Endpoint::Bind
 * @endcode
 *
 * ## 入站路径
 *
 * LinkEndpoint → NIC::DeliverNetworkPacket → ipv4::HandlePacket
 *   → Stack::DeliverTransportPacket → TransportDemuxer → Endpoint
 *
 * ## 出站路径
 *
 * Endpoint → ipv4::SendPacket → LinkEndpoint::WritePacket
 *
 * @see include/netstack/stack/stack.hh
 * @see docs/refer-arch.md
 */

#include "netstack/stack/stack.hh"

#include "netstack/header/ipv4.hh"
#include "netstack/net/ipv4/protocol.hh"

namespace netstack::stack {

Stack::Stack() = default;

/**
 * @brief 注册网络层协议（M0 起仅 IPv4）。
 *
 * 若注册的是 ipv4::Protocol，自动 `SetTransportDispatcherIfUnset(this)`，
 * 使 IPv4 剥头后能调用 DeliverTransportPacket。
 * 对已存在的 NIC 也会 RegisterProtocol，支持先 CreateNIC 后
 * AddNetworkProtocol。
 */
void Stack::AddNetworkProtocol(std::unique_ptr<NetworkProtocol> protocol) {
  const auto number = protocol->Number();
  protocols_[number] = std::move(protocol);
  NetworkProtocol* raw = protocols_[number].get();

  if (auto* ipv4 = dynamic_cast<net::ipv4::Protocol*>(raw)) {
    ipv4->SetTransportDispatcherIfUnset(this);
  }

  for (auto& [id, nic] : nics_) {
    (void)id;
    nic->RegisterProtocol(number, raw);
  }
}

/** @brief 注册传输层协议处理器（TCP/UDP），由 demuxer 按端口号分发。 */
void Stack::AddTransportProtocol(std::unique_ptr<TransportProtocol> protocol) {
  demuxer_.AddProtocol(std::move(protocol));
}

/**
 * @brief 创建 NIC 并 Attach 链路 endpoint。
 *
 * CreateNIC 内部构造 NIC，link_ep->Attach(NIC) 建立入站回调。
 * 返回 NICID 供 AddAddress / Listen / Bind 使用。
 */
NICID Stack::CreateNIC(std::unique_ptr<LinkEndpoint> link_ep) {
  const NICID id = next_nic_id_++;
  auto nic = std::make_unique<NIC>(id, std::move(link_ep));
  for (const auto& [number, proto] : protocols_) {
    nic->RegisterProtocol(number, proto.get());
  }
  nics_[id] = std::move(nic);
  nic_addresses_[id] = {};
  return id;
}

/**
 * @brief 登记「本 NIC 拥有的 IPv4 地址」。
 *
 * M3 教学栈用此告知协议栈「10.0.0.1 在本机」；与宿主机 `ip addr` 分工见
 * docs/m3.md。 可多次添加；完整路由表 SetRouteTable 未实现。
 */
StackResult Stack::AddAddress(NICID nic_id, IPv4Address addr) {
  if (GetNIC(nic_id) == nullptr) {
    return StackError{ErrorCode::kBadAddress};
  }
  nic_addresses_[nic_id].push_back(addr);
  return std::nullopt;
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

/** @brief 聚合 NIC 与 IPv4 协议计数，供 M0 验收与 fdbased 单测断言。 */
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

bool Stack::ReservePort(NetworkProtocolNumber net,
                        TransportProtocolNumber trans, uint16_t port) {
  return port_manager_.Reserve(net, trans, port);
}

void Stack::ReleasePort(NetworkProtocolNumber net,
                        TransportProtocolNumber trans, uint16_t port) {
  port_manager_.Release(net, trans, port);
}

/** @brief 监听套接字 / UDP Bind：登记 (local_ip, local_port) 键。 */
StackResult Stack::RegisterTransportEndpoint(NetworkProtocolNumber net,
                                             TransportProtocolNumber trans,
                                             TransportEndpointID id,
                                             TransportEndpoint* endpoint) {
  return demuxer_.RegisterEndpoint(net, trans, id, endpoint);
}

/** @brief TCP 已连接四元组：Connection 半连接与 ESTABLISHED 使用。 */
StackResult Stack::RegisterConnectedEndpoint(NetworkProtocolNumber net,
                                             TransportProtocolNumber trans,
                                             TransportEndpointID id,
                                             TransportEndpoint* endpoint) {
  return demuxer_.RegisterConnectedEndpoint(net, trans, id, endpoint);
}

void Stack::UnregisterConnectedEndpoint(NetworkProtocolNumber net,
                                        TransportProtocolNumber trans,
                                        TransportEndpointID id) {
  demuxer_.UnregisterConnectedEndpoint(net, trans, id);
}

/** @brief 网络层剥头后的传输层入口（由 ipv4::HandlePacket 调用）。 */
void Stack::DeliverTransportPacket(Route* route,
                                   TransportProtocolNumber protocol,
                                   PacketBuffer pkt) {
  if (route != nullptr && route->net_proto == 0) {
    route->net_proto = header::kIPv4ProtocolNumber;
  }
  (void)demuxer_.DeliverPacket(route, protocol, std::move(pkt));
}

}  // namespace netstack::stack
