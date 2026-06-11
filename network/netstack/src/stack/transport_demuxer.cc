/**
 * @file transport_demuxer.cc
 * @brief 传输层 demux（M1 UDP + M2+ TCP 双表）。
 *
 * ## TCP 查找（M2+）
 *
 * @code
 * 1. connected_endpoints_[四元组]  → tcp::Connection
 * 2. listen_endpoints_[local_ip, local_port]  → tcp::Listener（通常仅 SYN）
 * @endcode
 *
 * UDP 仍只使用 `listen_endpoints_`（历史名 `RegisterEndpoint`）。
 *
 * @see docs/m2+.md
 * @see docs/tcp-rfc793-states.md
 */

#include "netstack/header/ipv4.hh"
#include "netstack/header/tcp.hh"
#include "netstack/stack/transport.hh"

namespace netstack::stack {

void TransportDemuxer::AddProtocol(
    std::unique_ptr<TransportProtocol> protocol) {
  const auto number = protocol->Number();
  protocols_[number] = std::move(protocol);
}

size_t TransportDemuxer::BindKeyHash::operator()(const BindKey& k) const {
  size_t h = std::hash<uint16_t>{}(k.net) ^
             (std::hash<uint8_t>{}(k.trans) << 8) ^
             (std::hash<uint16_t>{}(k.port) << 16);
  for (uint8_t b : k.address) {
    h ^= std::hash<uint8_t>{}(b) + 0x9e3779b9 + (h << 6) + (h >> 2);
  }
  return h;
}

size_t TransportDemuxer::TupleKeyHash::operator()(const TupleKey& k) const {
  size_t h = BindKeyHash{}({k.net, k.trans, k.local_port, k.local_address});
  h ^= std::hash<uint16_t>{}(k.remote_port) + 0x9e3779b9 + (h << 6) + (h >> 2);
  for (uint8_t b : k.remote_address) {
    h ^= std::hash<uint8_t>{}(b) + 0x9e3779b9 + (h << 6) + (h >> 2);
  }
  return h;
}

TransportDemuxer::TupleKey TransportDemuxer::ToTupleKey(
    NetworkProtocolNumber net, TransportProtocolNumber trans,
    const TransportEndpointID& id) {
  return TupleKey{net,
                  trans,
                  id.local_port,
                  id.local_address,
                  id.remote_port,
                  id.remote_address};
}

StackResult TransportDemuxer::RegisterEndpoint(
    NetworkProtocolNumber net_proto, TransportProtocolNumber trans_proto,
    TransportEndpointID id, TransportEndpoint* endpoint) {
  if (endpoint == nullptr) {
    return StackError{ErrorCode::kInvalidEndpointState};
  }
  BindKey key{net_proto, trans_proto, id.local_port, id.local_address};
  if (listen_endpoints_.count(key) != 0) {
    return StackError{ErrorCode::kInvalidEndpointState};
  }
  listen_endpoints_[key] = endpoint;
  return std::nullopt;
}

StackResult TransportDemuxer::RegisterConnectedEndpoint(
    NetworkProtocolNumber net_proto, TransportProtocolNumber trans_proto,
    TransportEndpointID id, TransportEndpoint* endpoint) {
  if (endpoint == nullptr) {
    return StackError{ErrorCode::kInvalidEndpointState};
  }
  const TupleKey key = ToTupleKey(net_proto, trans_proto, id);
  if (connected_endpoints_.count(key) != 0) {
    return StackError{ErrorCode::kInvalidEndpointState};
  }
  connected_endpoints_[key] = endpoint;
  return std::nullopt;
}

void TransportDemuxer::UnregisterConnectedEndpoint(
    NetworkProtocolNumber net_proto, TransportProtocolNumber trans_proto,
    TransportEndpointID id) {
  const TupleKey key = ToTupleKey(net_proto, trans_proto, id);
  connected_endpoints_.erase(key);
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
  if (route->net_proto == 0) {
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
  if (!proto->ParsePorts(std::span<const uint8_t>(data.data(), data.size()),
                         src_port, dst_port)) {
    return false;
  }

  TransportEndpointID id{};
  id.local_port = dst_port;
  id.local_address = route->local_address;
  id.remote_port = src_port;
  id.remote_address = route->remote_address;

  if (trans_proto == header::kTCPProtocolNumber) {
    const TupleKey tkey = ToTupleKey(route->net_proto, trans_proto, id);
    if (auto conn_it = connected_endpoints_.find(tkey);
        conn_it != connected_endpoints_.end()) {
      conn_it->second->HandlePacket(route, id, std::move(pkt));
      return true;
    }
  }

  BindKey key{route->net_proto, trans_proto, id.local_port, id.local_address};
  auto ep_it = listen_endpoints_.find(key);
  if (ep_it == listen_endpoints_.end()) {
    proto->HandleUnknownDestinationPacket(route, id, std::move(pkt));
    return false;
  }

  ep_it->second->HandlePacket(route, id, std::move(pkt));
  return true;
}

}  // namespace netstack::stack
