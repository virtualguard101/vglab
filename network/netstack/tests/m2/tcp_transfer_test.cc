/**
 * @file tcp_transfer_test.cc
 * @brief M2 验收：握手后数据收发 + FIN 关闭（Listener + Connection）。
 *
 * @see docs/tcp-rfc793-states.md
 * @see ctest -R m2_tcp_transfer
 */

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

#include "netstack/header/tcp.hh"
#include "netstack/link/channel.hh"
#include "netstack/net/ipv4/protocol.hh"
#include "netstack/stack/stack.hh"
#include "netstack/transport/tcp/connection.hh"
#include "netstack/transport/tcp/listener.hh"
#include "netstack/transport/tcp/protocol.hh"
#include "tests/m2/packet_builder.hh"

using netstack::IPv4Address;
using netstack::LinkAddress;
using netstack::link::NewChannel;
using netstack::stack::PacketBuffer;
using netstack::stack::Stack;
using netstack::test::m2::MakeIpv4Tcp;
using netstack::test::m2::ParsedTcp;
using netstack::test::m2::ParseIpv4Tcp;
using netstack::test::m2::TcpSegmentFields;
using netstack::transport::tcp::Connection;
using netstack::transport::tcp::TcpState;

namespace {

Connection* CompleteHandshake(netstack::transport::tcp::Listener* listener,
                              netstack::link::ChannelEndpoint* ch) {
  const IPv4Address client{{10, 0, 0, 2}};
  const IPv4Address server_ip{{10, 0, 0, 1}};

  ch->InjectInbound(netstack::header::kIPv4ProtocolNumber,
                    PacketBuffer(MakeIpv4Tcp(
                        client, server_ip, 50000, 80,
                        TcpSegmentFields{.flags = static_cast<uint8_t>(
                                             netstack::header::TCPFlags::kSyn),
                                         .seq = 1000,
                                         .ack = 0})));
  auto syn_ack_out = ch->DrainOutbound();
  assert(syn_ack_out.size() == 1);
  ParsedTcp syn_ack{};
  assert(ParseIpv4Tcp(syn_ack_out.front().pkt.Data(), syn_ack));

  ch->InjectInbound(netstack::header::kIPv4ProtocolNumber,
                    PacketBuffer(MakeIpv4Tcp(
                        client, server_ip, 50000, 80,
                        TcpSegmentFields{.flags = static_cast<uint8_t>(
                                             netstack::header::TCPFlags::kAck),
                                         .seq = 1001,
                                         .ack = syn_ack.seq + 1})));

  auto* conn = listener->Accept();
  assert(conn != nullptr && conn->State() == TcpState::kEstablished);
  return conn;
}

void TestDataTransfer() {
  Stack s;
  s.AddNetworkProtocol(std::make_unique<netstack::net::ipv4::Protocol>());
  s.AddTransportProtocol(
      std::make_unique<netstack::transport::tcp::Protocol>());

  auto channel = NewChannel(16, 1500, LinkAddress{});
  auto* ch = dynamic_cast<netstack::link::ChannelEndpoint*>(channel.get());
  const auto nic_id = s.CreateNIC(std::move(channel));
  s.AddAddress(nic_id, IPv4Address{{10, 0, 0, 1}});

  netstack::transport::tcp::Listener listener(&s);
  assert(!listener.Listen(nic_id, IPv4Address{{10, 0, 0, 1}}, 80).has_value());

  Connection* server = CompleteHandshake(&listener, ch);
  const auto server_iss = server->Iss();

  const IPv4Address client{{10, 0, 0, 2}};
  const IPv4Address server_ip{{10, 0, 0, 1}};
  const std::vector<uint8_t> ping{'p', 'i', 'n', 'g'};

  ch->InjectInbound(netstack::header::kIPv4ProtocolNumber,
                    PacketBuffer(MakeIpv4Tcp(
                        client, server_ip, 50000, 80,
                        TcpSegmentFields{.flags = static_cast<uint8_t>(
                                             netstack::header::TCPFlags::kAck |
                                             netstack::header::TCPFlags::kPsh),
                                         .seq = 1001,
                                         .ack = server_iss + 1},
                        ping)));

  (void)ch->DrainOutbound();
  assert(server->Read() == ping);

  const std::string pong = "pong";
  assert(!server
              ->Write(std::span<const uint8_t>(
                  reinterpret_cast<const uint8_t*>(pong.data()), pong.size()))
              .has_value());

  auto data_out = ch->DrainOutbound();
  assert(data_out.size() == 1);
  ParsedTcp data_pkt{};
  assert(ParseIpv4Tcp(data_out.front().pkt.Data(), data_pkt));
  assert(data_pkt.payload == pong);
}

void TestFinClose() {
  Stack s;
  s.AddNetworkProtocol(std::make_unique<netstack::net::ipv4::Protocol>());
  s.AddTransportProtocol(
      std::make_unique<netstack::transport::tcp::Protocol>());

  auto channel = NewChannel(16, 1500, LinkAddress{});
  auto* ch = dynamic_cast<netstack::link::ChannelEndpoint*>(channel.get());
  const auto nic_id = s.CreateNIC(std::move(channel));
  s.AddAddress(nic_id, IPv4Address{{10, 0, 0, 1}});

  netstack::transport::tcp::Listener listener(&s);
  assert(!listener.Listen(nic_id, IPv4Address{{10, 0, 0, 1}}, 80).has_value());

  Connection* server = CompleteHandshake(&listener, ch);
  const auto server_iss = server->Iss();

  const IPv4Address client{{10, 0, 0, 2}};
  const IPv4Address server_ip{{10, 0, 0, 1}};

  ch->InjectInbound(netstack::header::kIPv4ProtocolNumber,
                    PacketBuffer(MakeIpv4Tcp(
                        client, server_ip, 50000, 80,
                        TcpSegmentFields{.flags = static_cast<uint8_t>(
                                             netstack::header::TCPFlags::kFin |
                                             netstack::header::TCPFlags::kAck),
                                         .seq = 1001,
                                         .ack = server_iss + 1})));
  (void)ch->DrainOutbound();
  assert(server->State() == TcpState::kCloseWait);

  server->Close();
  auto fin_out = ch->DrainOutbound();
  assert(fin_out.size() == 1);

  ch->InjectInbound(netstack::header::kIPv4ProtocolNumber,
                    PacketBuffer(MakeIpv4Tcp(
                        client, server_ip, 50000, 80,
                        TcpSegmentFields{.flags = static_cast<uint8_t>(
                                             netstack::header::TCPFlags::kAck),
                                         .seq = 1002,
                                         .ack = server_iss + 2})));

  assert(server->State() == TcpState::kClosed);
}

}  // namespace

int main() {
  TestDataTransfer();
  TestFinClose();
  return 0;
}
