/**
 * @file tcp_backlog_test.cc
 * @brief M2+ 验收：Listener backlog=1 时第二条 SYN 被丢弃。
 *
 * @see docs/m2+.md
 * @see ctest -R m2_tcp_backlog
 */

#include <cassert>

#include "netstack/header/tcp.hh"
#include "netstack/link/channel.hh"
#include "netstack/net/ipv4/protocol.hh"
#include "netstack/stack/stack.hh"
#include "netstack/transport/tcp/listener.hh"
#include "netstack/transport/tcp/protocol.hh"
#include "tests/m2/packet_builder.hh"

using netstack::IPv4Address;
using netstack::LinkAddress;
using netstack::link::NewChannel;
using netstack::stack::PacketBuffer;
using netstack::stack::Stack;
using netstack::test::m2::MakeIpv4Tcp;
using netstack::test::m2::TcpSegmentFields;

namespace {

void TestBacklogOne() {
  Stack s;
  s.AddNetworkProtocol(std::make_unique<netstack::net::ipv4::Protocol>());
  s.AddTransportProtocol(
      std::make_unique<netstack::transport::tcp::Protocol>());

  auto channel = NewChannel(16, 1500, LinkAddress{});
  auto* ch = dynamic_cast<netstack::link::ChannelEndpoint*>(channel.get());
  const auto nic_id = s.CreateNIC(std::move(channel));
  s.AddAddress(nic_id, IPv4Address{{10, 0, 0, 1}});

  netstack::transport::tcp::Listener listener(&s);
  assert(
      !listener.Listen(nic_id, IPv4Address{{10, 0, 0, 1}}, 80, 1).has_value());

  const IPv4Address server_ip{{10, 0, 0, 1}};

  ch->InjectInbound(netstack::header::kIPv4ProtocolNumber,
                    PacketBuffer(MakeIpv4Tcp(
                        IPv4Address{{10, 0, 0, 2}}, server_ip, 50000, 80,
                        TcpSegmentFields{.flags = static_cast<uint8_t>(
                                             netstack::header::TCPFlags::kSyn),
                                         .seq = 1000,
                                         .ack = 0})));
  assert(ch->DrainOutbound().size() == 1);

  ch->InjectInbound(netstack::header::kIPv4ProtocolNumber,
                    PacketBuffer(MakeIpv4Tcp(
                        IPv4Address{{10, 0, 0, 3}}, server_ip, 50001, 80,
                        TcpSegmentFields{.flags = static_cast<uint8_t>(
                                             netstack::header::TCPFlags::kSyn),
                                         .seq = 2000,
                                         .ack = 0})));
  assert(ch->DrainOutbound().empty());
}

}  // namespace

int main() {
  TestBacklogOne();
  return 0;
}
