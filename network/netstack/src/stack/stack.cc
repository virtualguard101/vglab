/**
 * @file stack.cc
 */

#include "netstack/stack/stack.hh"

#include "netstack/header/ipv4.hh"
#include "netstack/net/ipv4/protocol.hh"

namespace netstack::stack {

void Stack::AddNetworkProtocol(std::unique_ptr<NetworkProtocol> protocol) {
  const auto number = protocol->Number();
  protocols_[number] = std::move(protocol);
  NetworkProtocol* raw = protocols_[number].get();
  for (auto& [id, nic] : nics_) {
    (void)id;
    nic->RegisterProtocol(number, raw);
  }
}

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
