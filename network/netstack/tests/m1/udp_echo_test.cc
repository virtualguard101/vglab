/**
 * @file udp_echo_test.cc
 * @brief M1 验收：channel 注入 IPv4/UDP → echo → 出站队列。
 */

#include <cassert>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "netstack/header/ipv4.hh"
#include "netstack/header/udp.hh"
#include "netstack/link/channel.hh"
#include "netstack/net/ipv4/protocol.hh"
#include "netstack/stack/stack.hh"
#include "netstack/transport/udp/endpoint.hh"
#include "netstack/transport/udp/protocol.hh"

using netstack::IPv4Address;
using netstack::LinkAddress;
using netstack::header::IPv4Fields;
using netstack::header::IPv4Header;
using netstack::header::UDPFields;
using netstack::header::UDPHeader;
using netstack::link::NewChannel;
using netstack::net::ipv4::Protocol;
using netstack::stack::PacketBuffer;
using netstack::stack::Stack;

namespace {

std::vector<uint8_t> MakeIpv4Udp(IPv4Address src, IPv4Address dst,
                                 uint16_t sport, uint16_t dport,
                                 std::span<const uint8_t> payload) {
  const uint16_t udp_len =
      static_cast<uint16_t>(netstack::header::kUDPMinimumSize + payload.size());
  const uint16_t total_len =
      static_cast<uint16_t>(netstack::header::kIPv4MinimumSize + udp_len);

  std::vector<uint8_t> buf(total_len, 0);

  const size_t udp_off = netstack::header::kIPv4MinimumSize;
  UDPHeader udp(std::span<uint8_t>(buf.data() + udp_off, udp_len));
  UDPFields uf{};
  uf.src_port = sport;
  uf.dst_port = dport;
  uf.length = udp_len;
  uf.checksum = 0;
  udp.Encode(uf);
  std::copy(payload.begin(), payload.end(),
            buf.begin() + udp_off + netstack::header::kUDPMinimumSize);

  IPv4Header ip(std::span<uint8_t>(buf.data(), buf.size()));
  IPv4Fields fields{};
  fields.ihl = netstack::header::kIPv4MinimumSize;
  fields.total_length = total_len;
  fields.ttl = 64;
  fields.protocol = netstack::header::kUDPProtocolNumber;
  fields.src_addr = src;
  fields.dst_addr = dst;
  ip.Encode(fields);
  ip.SetChecksumField(0);
  ip.SetChecksumField(static_cast<uint16_t>(~ip.CalculateChecksum()));

  return buf;
}

bool ParseEcho(const std::vector<uint8_t>& frame, IPv4Address& dst,
               uint16_t& dport, std::string& payload) {
  if (frame.size() < netstack::header::kIPv4MinimumSize +
                        netstack::header::kUDPMinimumSize) {
    return false;
  }
  IPv4Header ip(std::span<uint8_t>(const_cast<uint8_t*>(frame.data()),
                                   frame.size()));
  if (!ip.IsValid(frame.size()) || !ip.IsChecksumValid()) {
    return false;
  }
  dst = ip.DestinationAddress();
  const auto hlen = ip.HeaderLength();
  UDPHeader udp(std::span<uint8_t>(const_cast<uint8_t*>(frame.data()) + hlen,
                                   frame.size() - hlen));
  if (!udp.IsValid(frame.size() - hlen)) {
    return false;
  }
  dport = udp.DestinationPort();
  const auto ulen = udp.Length();
  payload.assign(frame.begin() + hlen + netstack::header::kUDPMinimumSize,
                 frame.begin() + hlen + ulen);
  return true;
}

void TestUdpEchoOnChannel() {
  Stack s;
  s.AddNetworkProtocol(std::make_unique<Protocol>());
  s.AddTransportProtocol(
      std::make_unique<netstack::transport::udp::Protocol>());

  auto channel = NewChannel(16, 1500, LinkAddress{});
  auto* channel_raw =
      dynamic_cast<netstack::link::ChannelEndpoint*>(channel.get());
  assert(channel_raw != nullptr);

  const auto nic_id = s.CreateNIC(std::move(channel));
  s.AddAddress(nic_id, IPv4Address{{10, 0, 0, 1}});

  auto ep = std::make_unique<netstack::transport::udp::Endpoint>(&s);
  assert(!ep->Bind(nic_id, IPv4Address{{10, 0, 0, 1}}, 7).has_value());

  const std::vector<uint8_t> ping{'p', 'i', 'n', 'g'};
  channel_raw->InjectInbound(
      netstack::header::kIPv4ProtocolNumber,
      PacketBuffer(MakeIpv4Udp(IPv4Address{{10, 0, 0, 2}},
                               IPv4Address{{10, 0, 0, 1}}, 12345, 7, ping)));

  auto outbound = channel_raw->DrainOutbound();
  assert(outbound.size() == 1);

  IPv4Address dst{};
  uint16_t dport = 0;
  std::string echoed;
  assert(ParseEcho(outbound.front().pkt.Data(), dst, dport, echoed));
  assert(dst == IPv4Address({{10, 0, 0, 2}}));
  assert(dport == 12345);
  assert(echoed == "ping");
}

void TestNoListenerNoOutbound() {
  Stack s;
  s.AddNetworkProtocol(std::make_unique<Protocol>());
  s.AddTransportProtocol(
      std::make_unique<netstack::transport::udp::Protocol>());

  auto channel = NewChannel(16, 1500, LinkAddress{});
  auto* channel_raw =
      dynamic_cast<netstack::link::ChannelEndpoint*>(channel.get());
  const auto nic_id = s.CreateNIC(std::move(channel));
  s.AddAddress(nic_id, IPv4Address{{10, 0, 0, 1}});

  const std::vector<uint8_t> hi{'h', 'i'};
  channel_raw->InjectInbound(
      netstack::header::kIPv4ProtocolNumber,
      PacketBuffer(MakeIpv4Udp(IPv4Address{{10, 0, 0, 2}},
                               IPv4Address{{10, 0, 0, 1}}, 12345, 99, hi)));

  assert(channel_raw->DrainOutbound().empty());
}

}  // namespace

int main() {
  TestUdpEchoOnChannel();
  TestNoListenerNoOutbound();
  return 0;
}
