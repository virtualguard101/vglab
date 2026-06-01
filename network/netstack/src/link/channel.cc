/**
 * @file channel.cc
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

uint16_t ChannelEndpoint::MaxHeaderLength() const { return 0; }

LinkAddress ChannelEndpoint::LinkAddr() const { return link_addr_; }

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

void ChannelEndpoint::Attach(stack::NetworkDispatcher* dispatcher) {
  dispatcher_ = dispatcher;
}

bool ChannelEndpoint::IsAttached() const { return dispatcher_ != nullptr; }

void ChannelEndpoint::InjectInbound(stack::NetworkProtocolNumber protocol,
                                    stack::PacketBuffer pkt) {
  InjectLinkAddr(protocol, LinkAddress{}, std::move(pkt));
}

void ChannelEndpoint::InjectLinkAddr(stack::NetworkProtocolNumber protocol,
                                     LinkAddress remote,
                                     stack::PacketBuffer pkt) {
  if (dispatcher_ == nullptr) {
    return;
  }
  dispatcher_->DeliverNetworkPacket(*this, remote, LinkAddress{}, protocol,
                                    std::move(pkt));
}

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
