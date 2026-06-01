/**
 * @file ipv4_acceptance_test.cc
 * @brief M0 验收：合法/非法 IPv4 注入、统计与 transport 交付。
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

void TestValidIpv4Delivery() {
  M0Fixture f(true);
  f.Inject(netstack::header::kIPv4ProtocolNumber,
           MakeIPv4Packet(28, 17));  // 8-byte payload after header

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

void TestUnknownProtocol() {
  M0Fixture f(true);
  f.Inject(0x0806, MakeIPv4Packet(20, 17));  // ARP ethertype, not IPv4 handler

  assert(f.transport.Entries().empty());
  assert(f.ipv4->PacketsDelivered() == 0);

  const auto stats = f.stack.GetStats(f.nic_id);
  assert(stats.nic.unknown_protocol_rcvd == 1);
}

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
