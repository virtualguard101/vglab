/**
 * @file send.cc
 * @brief IPv4 出站路径。
 *
 * ## 封装步骤
 *
 * 1. total_length = 20 + len(l4_bytes)；
 * 2. 填写 IPv4Fields，计算头校验和（~CalculateChecksum）；
 * 3. 拷贝 l4 到偏移 20；
 * 4. `LinkEndpoint::WritePacket(..., kIPv4ProtocolNumber, pkt)`。
 *
 * loopback 会环回入站；**channel** 则只入 outbound 队列（M1 测试用）。
 *
 * @see include/netstack/net/ipv4/send.hh
 */

#include "netstack/net/ipv4/send.hh"

#include <algorithm>

#include "netstack/header/ipv4.hh"
#include "netstack/stack/stack.hh"

namespace netstack::net::ipv4 {

namespace {

IPv4Address FromStackAddress(const stack::Address& addr) {
  IPv4Address out{};
  if (addr.size() >= 4) {
    for (size_t i = 0; i < 4; ++i) {
      out.octets[i] = addr[i];
    }
  }
  return out;
}

}  // namespace

stack::StackResult SendPacket(stack::Stack& stack, stack::NICID nic_id,
                              stack::Route& route, uint8_t ip_protocol,
                              std::vector<uint8_t> l4_bytes) {
  auto* nic = stack.GetNIC(nic_id);
  if (nic == nullptr) {
    return stack::StackError{stack::ErrorCode::kBadAddress};
  }

  const auto src = FromStackAddress(route.local_address);
  const auto dst = FromStackAddress(route.remote_address);

  const uint16_t total_length =
      static_cast<uint16_t>(header::kIPv4MinimumSize + l4_bytes.size());
  std::vector<uint8_t> buf(total_length, 0);

  header::IPv4Header ip(std::span<uint8_t>(buf.data(), buf.size()));
  header::IPv4Fields fields{};
  fields.ihl = header::kIPv4MinimumSize;
  fields.total_length = total_length;
  fields.ttl = 64;
  fields.protocol = ip_protocol;
  fields.src_addr = src;
  fields.dst_addr = dst;
  ip.Encode(fields);
  ip.SetChecksumField(0);
  ip.SetChecksumField(static_cast<uint16_t>(~ip.CalculateChecksum()));

  std::copy(l4_bytes.begin(), l4_bytes.end(),
            buf.begin() + header::kIPv4MinimumSize);

  stack::PacketBuffer pkt(std::move(buf));
  return nic->GetLinkEndpoint().WritePacket(
      &route, nullptr, header::kIPv4ProtocolNumber, std::move(pkt));
}

}  // namespace netstack::net::ipv4
