/**
 * @file tcp_transfer_test.cc
 * @brief M2 验收：握手后单段数据收发 + FIN 关闭（channel）。
 *
 * ## 学习目标
 *
 * 1. `CompleteHandshake` 复用三次握手脚本；
 * 2. 注入 PSH+ACK 数据段 → `Read()` 取载荷 → `Write()` 回射；
 * 3. 对端 FIN → `CLOSE_WAIT` → `Close()` 发 FIN → ACK → `CLOSED`。
 *
 * ## TestDataTransfer 序列号约定
 *
 * 握手后：client seq=1001，server iss=100000，snd_nxt=100001。
 * 数据段 seq 必须从 1001 起（与 rcv_nxt_ 一致），否则 M2 会丢弃。
 *
 * ## RFC 793 路径（Figure 6）
 *
 * - `TestDataTransfer`：ESTABLISHED 上 RCV 数据 + ACK；出站 PSH|ACK
 * - `TestFinClose`：ESTABLISHED → CLOSE-WAIT → LAST-ACK → CLOSED
 *
 * @see docs/tcp-rfc793-states.md
 * @see docs/m2.md
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
#include "netstack/transport/tcp/endpoint.hh"
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
using netstack::transport::tcp::TcpState;

namespace {

/** @brief 注入 SYN + ACK，使 server 进入 ESTABLISHED。 */
void CompleteHandshake(netstack::link::ChannelEndpoint* ch,
                       netstack::transport::tcp::Endpoint* server) {
  const IPv4Address client{{10, 0, 0, 2}};
  const IPv4Address server_ip{{10, 0, 0, 1}};

  ch->InjectInbound(netstack::header::kIPv4ProtocolNumber,
                    PacketBuffer(MakeIpv4Tcp(
                        client, server_ip, 50000, 80,
                        TcpSegmentFields{.flags = static_cast<uint8_t>(
                                             netstack::header::TCPFlags::kSyn),
                                         .seq = 1000,
                                         .ack = 0})));
  (void)ch->DrainOutbound();

  ch->InjectInbound(netstack::header::kIPv4ProtocolNumber,
                    PacketBuffer(MakeIpv4Tcp(
                        client, server_ip, 50000, 80,
                        TcpSegmentFields{.flags = static_cast<uint8_t>(
                                             netstack::header::TCPFlags::kAck),
                                         .seq = 1001,
                                         .ack = server->Iss() + 1})));
  assert(server->State() == TcpState::kEstablished);
}

void TestDataTransfer() {
  Stack s;
  s.AddNetworkProtocol(std::make_unique<netstack::net::ipv4::Protocol>());
  s.AddTransportProtocol(
      std::make_unique<netstack::transport::tcp::Protocol>());

  auto channel = NewChannel(16, 1500, LinkAddress{});
  auto* ch = dynamic_cast<netstack::link::ChannelEndpoint*>(channel.get());
  assert(ch != nullptr);

  const auto nic_id = s.CreateNIC(std::move(channel));
  s.AddAddress(nic_id, IPv4Address{{10, 0, 0, 1}});

  auto server = std::make_unique<netstack::transport::tcp::Endpoint>(&s);
  assert(!server->Listen(nic_id, IPv4Address{{10, 0, 0, 1}}, 80).has_value());

  CompleteHandshake(ch, server.get());

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
                                         .ack = server->Iss() + 1},
                        ping)));

  // 服务端应回纯 ACK（无载荷）
  auto ack_only = ch->DrainOutbound();
  assert(ack_only.size() == 1);
  ParsedTcp ack_pkt{};
  assert(ParseIpv4Tcp(ack_only.front().pkt.Data(), ack_pkt));
  assert(ack_pkt.payload.empty());
  assert((ack_pkt.flags &
          static_cast<uint8_t>(netstack::header::TCPFlags::kAck)) != 0);

  auto received = server->Read();
  assert(received == ping);

  const std::string pong = "pong";
  assert(!server
              ->Write(std::span<const uint8_t>(
                  reinterpret_cast<const uint8_t*>(pong.data()), pong.size()))
              .has_value());

  auto data_out = ch->DrainOutbound();
  assert(data_out.size() == 1);
  ParsedTcp data_pkt{};
  assert(ParseIpv4Tcp(data_out.front().pkt.Data(), data_pkt));
  assert(data_pkt.dst == client);
  assert(data_pkt.dport == 50000);
  assert(data_pkt.payload == pong);
  assert((data_pkt.flags &
          static_cast<uint8_t>(netstack::header::TCPFlags::kPsh)) != 0);
}

/** @brief 被动关闭：对端 FIN → 本端 Close() → 对端 ACK → CLOSED。 */
void TestFinClose() {
  Stack s;
  s.AddNetworkProtocol(std::make_unique<netstack::net::ipv4::Protocol>());
  s.AddTransportProtocol(
      std::make_unique<netstack::transport::tcp::Protocol>());

  auto channel = NewChannel(16, 1500, LinkAddress{});
  auto* ch = dynamic_cast<netstack::link::ChannelEndpoint*>(channel.get());
  assert(ch != nullptr);

  const auto nic_id = s.CreateNIC(std::move(channel));
  s.AddAddress(nic_id, IPv4Address{{10, 0, 0, 1}});

  auto server = std::make_unique<netstack::transport::tcp::Endpoint>(&s);
  assert(!server->Listen(nic_id, IPv4Address{{10, 0, 0, 1}}, 80).has_value());

  CompleteHandshake(ch, server.get());

  const IPv4Address client{{10, 0, 0, 2}};
  const IPv4Address server_ip{{10, 0, 0, 1}};
  const auto server_ack = server->Iss() + 1;

  ch->InjectInbound(netstack::header::kIPv4ProtocolNumber,
                    PacketBuffer(MakeIpv4Tcp(
                        client, server_ip, 50000, 80,
                        TcpSegmentFields{.flags = static_cast<uint8_t>(
                                             netstack::header::TCPFlags::kFin |
                                             netstack::header::TCPFlags::kAck),
                                         .seq = 1001,
                                         .ack = server_ack})));
  (void)ch->DrainOutbound();

  assert(server->State() == TcpState::kCloseWait);

  server->Close();
  auto fin_out = ch->DrainOutbound();
  assert(fin_out.size() == 1);
  ParsedTcp fin_pkt{};
  assert(ParseIpv4Tcp(fin_out.front().pkt.Data(), fin_pkt));
  assert((fin_pkt.flags &
          static_cast<uint8_t>(netstack::header::TCPFlags::kFin)) != 0);

  // 对端 ACK 我方 FIN（seq=iss+1，故 ack=iss+2）
  ch->InjectInbound(netstack::header::kIPv4ProtocolNumber,
                    PacketBuffer(MakeIpv4Tcp(
                        client, server_ip, 50000, 80,
                        TcpSegmentFields{.flags = static_cast<uint8_t>(
                                             netstack::header::TCPFlags::kAck),
                                         .seq = 1002,
                                         .ack = server->Iss() + 2})));

  assert(server->State() == TcpState::kClosed);
}

}  // namespace

int main() {
  TestDataTransfer();
  TestFinClose();
  return 0;
}
