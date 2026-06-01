/**
 * @file loopback_dispatch_test.cc
 * @brief M0：loopback → NIC → IPv4 → transport stub。
 */

#include <cassert>
#include <cstdint>
#include <vector>

#include "netstack/header/ipv4.hh"
#include "netstack/link/channel.hh"
#include "netstack/link/loopback.hh"
#include "netstack/net/ipv4/protocol.hh"
#include "netstack/stack/stack.hh"

using netstack::IPv4Address;
using netstack::LinkAddress;
using netstack::header::IPv4Fields;
using netstack::header::IPv4Header;
using netstack::link::NewChannel;
using netstack::link::NewLoopback;
using netstack::net::ipv4::Protocol;
using netstack::stack::PacketBuffer;
using netstack::stack::RecordingTransportDispatcher;
using netstack::stack::Stack;

namespace {

std::vector<uint8_t> MakeIPv4UdpPacket() {
  std::vector<uint8_t> buf(20, 0);
  IPv4Header hdr(buf);
  IPv4Fields fields{};
  fields.ihl = 20;
  fields.total_length = 20;
  fields.ttl = 64;
  fields.protocol = 17;
  fields.src_addr = IPv4Address{{10, 0, 0, 1}};
  fields.dst_addr = IPv4Address{{10, 0, 0, 2}};
  hdr.Encode(fields);
  hdr.SetChecksumField(0);
  hdr.SetChecksumField(~hdr.CalculateChecksum());
  return buf;
}

void TestLoopbackDispatch() {
  Stack s;
  auto ipv4 = std::make_unique<Protocol>();
  auto* ipv4_raw = ipv4.get();
  RecordingTransportDispatcher transport;
  ipv4->SetTransportDispatcher(&transport);
  s.AddNetworkProtocol(std::move(ipv4));

  const auto nic_id = s.CreateNIC(NewLoopback());
  auto* nic = s.GetNIC(nic_id);
  assert(nic != nullptr);

  auto pkt = PacketBuffer(MakeIPv4UdpPacket());
  const auto result = nic->GetLinkEndpoint().WritePacket(
      nullptr, nullptr, netstack::header::kIPv4ProtocolNumber, std::move(pkt));
  assert(!result.has_value());
  assert(ipv4_raw->PacketsDelivered() == 1);
  assert(transport.Entries().size() == 1);
  assert(transport.Entries()[0].protocol == 17);
}

void TestChannelInject() {
  Stack s;
  auto ipv4 = std::make_unique<Protocol>();
  RecordingTransportDispatcher transport;
  ipv4->SetTransportDispatcher(&transport);
  s.AddNetworkProtocol(std::move(ipv4));

  auto channel = NewChannel(16, 1500, LinkAddress{});
  auto* channel_raw =
      dynamic_cast<netstack::link::ChannelEndpoint*>(channel.get());
  assert(channel_raw != nullptr);

  const auto nic_id = s.CreateNIC(std::move(channel));
  (void)nic_id;

  channel_raw->InjectInbound(netstack::header::kIPv4ProtocolNumber,
                             PacketBuffer(MakeIPv4UdpPacket()));
  assert(transport.Entries().size() == 1);
}

}  // namespace

int main() {
  TestLoopbackDispatch();
  TestChannelInject();
  return 0;
}
