/**
 * @file nic.cc
 */

#include "netstack/stack/nic.hh"

namespace netstack::stack {

NIC::NIC(NICID id, std::unique_ptr<LinkEndpoint> link_ep)
    : id_(id), link_ep_(std::move(link_ep)) {
  link_ep_->Attach(this);
}

void NIC::RegisterProtocol(NetworkProtocolNumber number,
                           NetworkProtocol* protocol) {
  protocols_[number] = protocol;
}

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
    ++unknown_protocol_rcvd_;
    return;
  }

  NetworkProtocol* net_proto = it->second;
  if (pkt.Data().size() < static_cast<size_t>(net_proto->MinimumPacketSize())) {
    ++malformed_rcvd_;
    return;
  }

  Route route{};
  route.net_proto = protocol;
  net_proto->HandlePacket(&route, std::move(pkt));
}

}  // namespace netstack::stack
