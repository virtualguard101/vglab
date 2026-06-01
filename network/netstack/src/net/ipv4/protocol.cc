/**
 * @file protocol.cc
 * @brief IPv4 网络层入站处理（对标 references/tcpip/network/ipv4/ipv4.go）。
 *
 * ## HandlePacket 流水线（M0）
 *
 * 1. 长度与 `IsValid` 结构检查（版本、IHL、TotalLength）；
 * 2. **显式** `IsChecksumValid()`（教学栈比参考实现更严）；
 * 3. 拒绝分片（MF 或 FragmentOffset ≠ 0）；
 * 4. 剥 IPv4 头，将 transport payload 交给 TransportDispatcher；
 * 5. 递增 packets_delivered_ 或 malformed_packets_received_。
 *
 * @see include/netstack/net/ipv4/protocol.hh
 * @see docs/m0.md
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

/**
 * @brief 从仍以 IPv4 头开头的 packet 中解析源/目的地址（供路由/日志用）。
 */
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

/**
 * @brief IPv4 入站主路径；pkt.Data() 入参时以 IPv4
 * 头开始，返回前仅余传输层载荷。
 */
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
    // M0 不实现分片重组
    ++malformed_packets_received_;
    return;
  }

  const auto transport_proto =
      static_cast<stack::TransportProtocolNumber>(ip.Protocol());

  // 保存网络头副本供出站或调试；data 剥头后仅余 [hlen, tlen) 载荷
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
