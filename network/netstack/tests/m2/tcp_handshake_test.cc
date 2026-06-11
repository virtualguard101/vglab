/**
 * @file tcp_handshake_test.cc
 * @brief M2 验收：被动打开三次握手（channel）。
 *
 * ## 学习目标
 *
 * 1. 在 Stack 上注册 TCP 协议并 `Listen(80)`；
 * 2. 用 `MakeIpv4Tcp` 模拟客户端发 SYN；
 * 3. 从 `DrainOutbound` 断言 SYN-ACK（seq=iss_, ack=client_seq+1）；
 * 4. 注入第三次 ACK 后状态为 `ESTABLISHED`。
 *
 * ## 拓扑
 *
 * @code
 *  Inject: 10.0.0.2:50000 → 10.0.0.1:80  SYN seq=1000
 *  Outbound: 10.0.0.1:80 → 10.0.0.2:50000  SYN|ACK seq=100000 ack=1001
 *  Inject: ACK seq=1001 ack=100001
 *  State: ESTABLISHED
 * @endcode
 *
 * ## RFC 793 路径（Figure 6）
 *
 * CLOSED → LISTEN → SYN-RECEIVED → ESTABLISHED
 * （被动打开三次握手，见 docs/tcp-rfc793-states.md）
 *
 * @see docs/tcp-rfc793-states.md
 * @see docs/m2.md
 * @see ctest -R m2_tcp_handshake
 */

#include <cassert>
#include <cstdint>

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

void TestPassiveHandshake() {
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
  assert(server->State() == TcpState::kListen);

  const IPv4Address client{{10, 0, 0, 2}};
  const IPv4Address server_ip{{10, 0, 0, 1}};

  // 第一次握手：客户端 SYN
  ch->InjectInbound(netstack::header::kIPv4ProtocolNumber,
                    PacketBuffer(MakeIpv4Tcp(
                        client, server_ip, 50000, 80,
                        TcpSegmentFields{.flags = static_cast<uint8_t>(
                                             netstack::header::TCPFlags::kSyn),
                                         .seq = 1000,
                                         .ack = 0})));

  auto outbound = ch->DrainOutbound();
  assert(outbound.size() == 1);

  // 第二次握手：服务端 SYN-ACK
  ParsedTcp syn_ack{};
  assert(ParseIpv4Tcp(outbound.front().pkt.Data(), syn_ack));
  assert(syn_ack.dst == client);
  assert(syn_ack.dport == 50000);
  assert((syn_ack.flags &
          static_cast<uint8_t>(netstack::header::TCPFlags::kSyn)) != 0);
  assert((syn_ack.flags &
          static_cast<uint8_t>(netstack::header::TCPFlags::kAck)) != 0);
  assert(syn_ack.seq == server->Iss());
  assert(syn_ack.ack == 1001);

  // 第三次握手：客户端 ACK
  ch->InjectInbound(netstack::header::kIPv4ProtocolNumber,
                    PacketBuffer(MakeIpv4Tcp(
                        client, server_ip, 50000, 80,
                        TcpSegmentFields{.flags = static_cast<uint8_t>(
                                             netstack::header::TCPFlags::kAck),
                                         .seq = 1001,
                                         .ack = server->Iss() + 1})));

  assert(server->State() == TcpState::kEstablished);
  assert(ch->DrainOutbound().empty());
}

}  // namespace

int main() {
  TestPassiveHandshake();
  return 0;
}
