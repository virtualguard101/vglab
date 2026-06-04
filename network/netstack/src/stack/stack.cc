/**
 * @file stack.cc
 */

#include "netstack/stack/stack.hh"

#include "netstack/header/ipv4.hh"
#include "netstack/net/ipv4/protocol.hh"

namespace netstack::stack {

Stack::Stack() = default;

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

void Stack::AddTransportProtocol(std::unique_ptr<TransportProtocol> protocol) {
  demuxer_.AddProtocol(std::move(protocol));
}

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

StackResult Stack::RegisterTransportEndpoint(
    NetworkProtocolNumber net, TransportProtocolNumber trans,
    TransportEndpointID id, TransportEndpoint* endpoint) {
  return demuxer_.RegisterEndpoint(net, trans, id, endpoint);
}

void Stack::DeliverTransportPacket(Route* route,
                                   TransportProtocolNumber protocol,
                                   PacketBuffer pkt) {
  if (route != nullptr && route->net_proto == 0) {
    route->net_proto = header::kIPv4ProtocolNumber;
  }
  (void)demuxer_.DeliverPacket(route, protocol, std::move(pkt));
}

}  // namespace netstack::stack
