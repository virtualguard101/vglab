/**
 * @file transport_demuxer.cc
 * @brief 传输层 demux（M1 精简版）。
 */

#include "netstack/stack/transport.hh"

#include "netstack/header/ipv4.hh"

namespace netstack::stack {

void TransportDemuxer::AddProtocol(std::unique_ptr<TransportProtocol> protocol) {
  const auto number = protocol->Number();
  protocols_[number] = std::move(protocol);
}

size_t TransportDemuxer::BindKeyHash::operator()(const BindKey& k) const {
  size_t h = std::hash<uint16_t>{}(k.net) ^ (std::hash<uint8_t>{}(k.trans) << 8) ^
             (std::hash<uint16_t>{}(k.port) << 16);
  for (uint8_t b : k.address) {
    h ^= std::hash<uint8_t>{}(b) + 0x9e3779b9 + (h << 6) + (h >> 2);
  }
  return h;
}

StackResult TransportDemuxer::RegisterEndpoint(
    NetworkProtocolNumber net_proto, TransportProtocolNumber trans_proto,
    TransportEndpointID id, TransportEndpoint* endpoint) {
  if (endpoint == nullptr) {
    return StackError{ErrorCode::kInvalidEndpointState};
  }
  BindKey key{net_proto, trans_proto, id.local_port, id.local_address};
  if (endpoints_.count(key) != 0) {
    return StackError{ErrorCode::kInvalidEndpointState};
  }
  endpoints_[key] = endpoint;
  return std::nullopt;
}

TransportProtocol* TransportDemuxer::FindProtocol(
    TransportProtocolNumber number) {
  auto it = protocols_.find(number);
  return it == protocols_.end() ? nullptr : it->second.get();
}

const TransportProtocol* TransportDemuxer::FindProtocol(
    TransportProtocolNumber number) const {
  auto it = protocols_.find(number);
  return it == protocols_.end() ? nullptr : it->second.get();
}

bool TransportDemuxer::DeliverPacket(Route* route,
                                     TransportProtocolNumber trans_proto,
                                     PacketBuffer pkt) {
  if (route == nullptr) {
    return false;
  }
  const NetworkProtocolNumber net_proto = route->net_proto;
  if (net_proto == 0) {
    route->net_proto = header::kIPv4ProtocolNumber;
  }

  auto it = protocols_.find(trans_proto);
  if (it == protocols_.end()) {
    return false;
  }
  TransportProtocol* proto = it->second.get();

  auto& data = pkt.Data();
  if (data.size() < static_cast<size_t>(proto->MinimumPacketSize())) {
    return false;
  }

  uint16_t src_port = 0;
  uint16_t dst_port = 0;
  if (!proto->ParsePorts(
          std::span<const uint8_t>(data.data(), data.size()), src_port,
          dst_port)) {
    return false;
  }

  TransportEndpointID id{};
  id.local_port = dst_port;
  id.local_address = route->local_address;
  id.remote_port = src_port;
  id.remote_address = route->remote_address;

  BindKey key{route->net_proto, trans_proto, id.local_port, id.local_address};
  auto ep_it = endpoints_.find(key);
  if (ep_it == endpoints_.end()) {
    proto->HandleUnknownDestinationPacket(route, id, std::move(pkt));
    return false;
  }

  ep_it->second->HandlePacket(route, id, std::move(pkt));
  return true;
}

}  // namespace netstack::stack
