/**
 * @file tcp_connect_test.cc
 * @brief M2+ 验收：Connect 主动打开 + Listener Accept（同栈 channel 环回）。
 *
 * ## RFC 793 路径
 *
 * 客户端：CLOSED → SYN-SENT → ESTABLISHED
 * 服务端：LISTEN → SYN-RECEIVED → ESTABLISHED
 *
 * ## 测试技巧
 *
 * 单 Stack + channel：`Connect()` 的 SYN 进入 outbound，再 `InjectInbound`
 * 同一帧驱动 Listener；SYN-ACK 同样 reinject 完成客户端握手。
 *
 * @see docs/m2+.md
 * @see ctest -R m2_tcp_connect
 */

#include <cassert>
#include <vector>

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
using netstack::stack::Stack;
using netstack::test::m2::ParsedTcp;
using netstack::test::m2::ParseIpv4Tcp;
using netstack::transport::tcp::Connection;
using netstack::transport::tcp::TcpState;

namespace {

/** @brief 将出站队列中的包全部注入入站，直到握手完成且 Accept 可用。 */
Connection* PumpConnectHandshake(Connection* client,
                                 netstack::transport::tcp::Listener* listener,
                                 netstack::link::ChannelEndpoint* ch) {
  for (int round = 0; round < 12; ++round) {
    if (client->State() == TcpState::kEstablished) {
      if (Connection* accepted = listener->Accept()) {
        return accepted;
      }
    }
    auto out = ch->DrainOutbound();
    if (out.empty()) {
      continue;
    }
    for (auto& item : out) {
      ch->InjectInbound(item.proto, std::move(item.pkt));
    }
  }
  assert(client->State() == TcpState::kEstablished);
  Connection* server = listener->Accept();
  assert(server != nullptr);
  return server;
}

void TestActiveConnect() {
  Stack s;
  s.AddNetworkProtocol(std::make_unique<netstack::net::ipv4::Protocol>());
  s.AddTransportProtocol(
      std::make_unique<netstack::transport::tcp::Protocol>());

  auto channel = NewChannel(16, 1500, LinkAddress{});
  auto* ch = dynamic_cast<netstack::link::ChannelEndpoint*>(channel.get());
  const auto nic_id = s.CreateNIC(std::move(channel));
  s.AddAddress(nic_id, IPv4Address{{10, 0, 0, 1}});
  s.AddAddress(nic_id, IPv4Address{{10, 0, 0, 2}});

  netstack::transport::tcp::Listener listener(&s);
  assert(!listener.Listen(nic_id, IPv4Address{{10, 0, 0, 1}}, 80).has_value());

  Connection client(&s);
  assert(!client
              .Connect(nic_id, IPv4Address{{10, 0, 0, 2}}, 50000,
                       IPv4Address{{10, 0, 0, 1}}, 80)
              .has_value());
  assert(client.State() == TcpState::kSynSent);

  Connection* server = PumpConnectHandshake(&client, &listener, ch);
  assert(client.State() == TcpState::kEstablished);
  assert(server->State() == TcpState::kEstablished);
}

void TestConnectAndTransfer() {
  Stack s;
  s.AddNetworkProtocol(std::make_unique<netstack::net::ipv4::Protocol>());
  s.AddTransportProtocol(
      std::make_unique<netstack::transport::tcp::Protocol>());

  auto channel = NewChannel(16, 1500, LinkAddress{});
  auto* ch = dynamic_cast<netstack::link::ChannelEndpoint*>(channel.get());
  const auto nic_id = s.CreateNIC(std::move(channel));
  s.AddAddress(nic_id, IPv4Address{{10, 0, 0, 1}});
  s.AddAddress(nic_id, IPv4Address{{10, 0, 0, 2}});

  netstack::transport::tcp::Listener listener(&s);
  assert(!listener.Listen(nic_id, IPv4Address{{10, 0, 0, 1}}, 80).has_value());

  Connection client(&s);
  assert(!client
              .Connect(nic_id, IPv4Address{{10, 0, 0, 2}}, 50001,
                       IPv4Address{{10, 0, 0, 1}}, 80)
              .has_value());
  Connection* server = PumpConnectHandshake(&client, &listener, ch);

  const std::vector<uint8_t> hi{'h', 'i'};
  assert(!client.Write(hi).has_value());
  auto data_out = ch->DrainOutbound();
  assert(data_out.size() == 1);
  ch->InjectInbound(data_out.front().proto, std::move(data_out.front().pkt));
  (void)ch->DrainOutbound();

  assert(server->Read() == hi);

  const std::string bye = "bye";
  assert(!server
              ->Write(std::span<const uint8_t>(
                  reinterpret_cast<const uint8_t*>(bye.data()), bye.size()))
              .has_value());
  auto out = ch->DrainOutbound();
  assert(out.size() == 1);
  ParsedTcp pkt{};
  assert(ParseIpv4Tcp(out.front().pkt.Data(), pkt));
  assert(pkt.payload == bye);
}

}  // namespace

int main() {
  TestActiveConnect();
  TestConnectAndTransfer();
  return 0;
}
