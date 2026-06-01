/**
 * @file ipv4_acceptance_test.cc
 * @brief M0 验收测试：合法/非法 IPv4 注入、统计与 transport 交付。
 *
 * ## 对应文档
 *
 * 覆盖 docs/m0.md「验收 checklist」中的入站场景，使用 channel 为主、
 * loopback 为辅。
 *
 * ## 统计分层
 *
 * | 丢弃原因              | 计数位置                    |
 * |-----------------------|-----------------------------|
 * | 未知 L3 协议号        | NIC::unknown_protocol_rcvd_ |
 * | 短于 MinimumPacketSize| NIC::malformed_rcvd_          |
 * | IPv4 校验/分片等      | ipv4::malformed_packets_*     |
 * | 成功交付传输层        | ipv4::packets_delivered_      |
 *
 * @see docs/m0.md
 * @see ctest -R m0_ipv4_acceptance
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

/**
 * @brief 构造指定 TotalLength 与 Protocol 字段的 IPv4 包（含正确头校验和）。
 *
 * @param total_length 整包长度（头+载荷），须 ≥ 20。
 * @param protocol 上层协议号，如 17=UDP。
 */
std::vector<uint8_t> MakeIPv4Packet(uint16_t total_length, uint8_t protocol) {
  std::vector<uint8_t> buf(total_length, 0);
  IPv4Header hdr(buf);
  IPv4Fields fields{};
  fields.ihl = 20;
  fields.total_length = total_length;
  fields.ttl = 64;
  fields.protocol = protocol;
  fields.src_addr = IPv4Address{{10, 0, 0, 1}};
  fields.dst_addr = IPv4Address{{10, 0, 0, 2}};
  hdr.Encode(fields);
  hdr.SetChecksumField(0);
  hdr.SetChecksumField(~hdr.CalculateChecksum());
  return buf;
}

/**
 * @brief 测试夹具：组装 Stack + IPv4 + RecordingTransportDispatcher + 链路。
 *
 * @param use_channel true 用 channel（InjectInbound）；false 用 loopback。
 */
struct M0Fixture {
  Stack stack;
  Protocol* ipv4{nullptr};
  RecordingTransportDispatcher transport;
  netstack::link::ChannelEndpoint* channel{nullptr};
  netstack::stack::NICID nic_id{};

  explicit M0Fixture(bool use_channel) {
    auto proto = std::make_unique<Protocol>();
    ipv4 = proto.get();
    proto->SetTransportDispatcher(&transport);
    stack.AddNetworkProtocol(std::move(proto));

    if (use_channel) {
      auto ch = NewChannel(16, 1500, LinkAddress{});
      channel = dynamic_cast<netstack::link::ChannelEndpoint*>(ch.get());
      assert(channel != nullptr);
      nic_id = stack.CreateNIC(std::move(ch));
    } else {
      nic_id = stack.CreateNIC(NewLoopback());
    }
  }

  /** @brief 按链路类型选择 InjectInbound 或 loopback WritePacket。 */
  void Inject(netstack::stack::NetworkProtocolNumber proto,
              std::vector<uint8_t> bytes) {
    if (channel != nullptr) {
      channel->InjectInbound(proto, PacketBuffer(std::move(bytes)));
      return;
    }
    auto* nic = stack.GetNIC(nic_id);
    assert(nic != nullptr);
    const auto result = nic->GetLinkEndpoint().WritePacket(
        nullptr, nullptr, proto, PacketBuffer(std::move(bytes)));
    assert(!result.has_value());
  }
};

/**
 * @brief 合法包：28 字节总长 → 剥头后 8 字节载荷交付给 stub（protocol=17）。
 */
void TestValidIpv4Delivery() {
  M0Fixture f(true);
  f.Inject(netstack::header::kIPv4ProtocolNumber, MakeIPv4Packet(28, 17));

  assert(f.ipv4->PacketsDelivered() == 1);
  assert(f.transport.Entries().size() == 1);
  assert(f.transport.Entries()[0].protocol == 17);
  assert(f.transport.Entries()[0].payload_size == 8);

  const auto stats = f.stack.GetStats(f.nic_id);
  assert(stats.nic.rx_packets == 1);
  assert(stats.nic.rx_bytes == 28);
  assert(stats.ipv4_packets_delivered == 1);
  assert(stats.ipv4_malformed_received == 0);
}

/**
 * @brief 破坏校验和（翻转 checksum 字段某一字节）→ IPv4 层丢弃。
 */
void TestBadChecksumDropped() {
  M0Fixture f(true);
  auto pkt = MakeIPv4Packet(20, 17);
  pkt[10] ^= 0x01;
  f.Inject(netstack::header::kIPv4ProtocolNumber, std::move(pkt));

  assert(f.transport.Entries().empty());
  assert(f.ipv4->PacketsDelivered() == 0);
  assert(f.ipv4->MalformedPacketsReceived() == 1);

  const auto stats = f.stack.GetStats(f.nic_id);
  assert(stats.ipv4_malformed_received == 1);
  assert(stats.nic.malformed_rcvd == 0);
}

/**
 * @brief 10 字节包短于 IPv4 MinimumPacketSize(20) → NIC 层丢弃。
 */
void TestShortPacketDropped() {
  M0Fixture f(true);
  std::vector<uint8_t> short_pkt(10, 0);
  f.Inject(netstack::header::kIPv4ProtocolNumber, std::move(short_pkt));

  assert(f.transport.Entries().empty());
  assert(f.ipv4->MalformedPacketsReceived() == 0);
  assert(f.ipv4->PacketsDelivered() == 0);

  const auto stats = f.stack.GetStats(f.nic_id);
  assert(stats.nic.malformed_rcvd == 1);
}

/**
 * @brief 注入 ARP EtherType(0x0806) 而栈只注册 IPv4 → unknown_protocol。
 */
void TestUnknownProtocol() {
  M0Fixture f(true);
  f.Inject(0x0806, MakeIPv4Packet(20, 17));

  assert(f.transport.Entries().empty());
  assert(f.ipv4->PacketsDelivered() == 0);

  const auto stats = f.stack.GetStats(f.nic_id);
  assert(stats.nic.unknown_protocol_rcvd == 1);
}

/** @brief loopback 路径下合法最小 IPv4 包也能到达 stub。 */
void TestLoopbackValidPath() {
  M0Fixture f(false);
  f.Inject(netstack::header::kIPv4ProtocolNumber, MakeIPv4Packet(20, 17));
  assert(f.ipv4->PacketsDelivered() == 1);
  assert(f.transport.Entries().size() == 1);
}

}  // namespace

int main() {
  TestValidIpv4Delivery();
  TestBadChecksumDropped();
  TestShortPacketDropped();
  TestUnknownProtocol();
  TestLoopbackValidPath();
  return 0;
}
