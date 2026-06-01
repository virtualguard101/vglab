/**
 * @file protocol.cc
 * @brief IPv4 入站处理（references/tcpip/network/ipv4/ipv4.go）。
 */

#include "netstack/net/ipv4/protocol.hh"

#include "netstack/header/ipv4.hh"

namespace netstack::net::ipv4 {

stack::NetworkProtocolNumber Protocol::Number() const {
  return header::kIPv4ProtocolNumber;
}

int Protocol::MinimumPacketSize() const {
  return static_cast<int>(header::kIPv4MinimumSize);
}

void Protocol::ParseAddresses(const std::vector<uint8_t>& packet,
                              stack::Address& src, stack::Address& dst) const {
  if (packet.size() < header::kIPv4MinimumSize) {
    src.clear();
    dst.clear();
    return;
  }
  header::IPv4Header hdr(
      std::span<uint8_t>(const_cast<uint8_t*>(packet.data()), packet.size()));
  const auto s = hdr.SourceAddress();
  const auto d = hdr.DestinationAddress();
  src.assign(s.octets.begin(), s.octets.end());
  dst.assign(d.octets.begin(), d.octets.end());
}

void Protocol::HandlePacket(stack::Route* route, stack::PacketBuffer pkt) {
  (void)route;
  auto& data = pkt.Data();
  if (data.size() < header::kIPv4MinimumSize) {
    ++malformed_packets_received_;
    return;
  }

  header::IPv4Header ip(std::span<uint8_t>(data.data(), data.size()));
  if (!ip.IsValid(data.size())) {
    ++malformed_packets_received_;
    return;
  }
  if (!ip.IsChecksumValid()) {
    ++malformed_packets_received_;
    return;
  }

  const auto hlen = ip.HeaderLength();
  const auto tlen = ip.TotalLength();
  if (tlen > data.size()) {
    ++malformed_packets_received_;
    return;
  }

  const uint8_t flags = ip.Flags();
  const uint16_t frag_off = ip.FragmentOffset();
  if ((flags & static_cast<uint8_t>(header::IPv4Flags::MoreFragments)) != 0 ||
      frag_off != 0) {
    ++malformed_packets_received_;
    return;
  }

  const auto transport_proto =
      static_cast<stack::TransportProtocolNumber>(ip.Protocol());

  // 保存网络头副本；载荷为 [hlen, tlen)。
  pkt.NetworkHeader().assign(data.begin(), data.begin() + hlen);
  data.erase(data.begin(), data.begin() + hlen);
  if (data.size() > tlen - hlen) {
    data.resize(tlen - hlen);
  }
  if (transport_dispatcher_ == nullptr) {
    return;
  }
  ++packets_delivered_;
  transport_dispatcher_->DeliverTransportPacket(route, transport_proto,
                                                std::move(pkt));
}

}  // namespace netstack::net::ipv4
