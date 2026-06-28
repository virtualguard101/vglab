/**
 * @file fdbased_test.cc
 * @brief FdEndpoint 读写交付（socketpair 模拟，不依赖 TUN/root）。
 *
 * @see docs/m3.md
 */

#include "netstack/link/fdbased.hh"

#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cstdint>
#include <vector>

#include "netstack/address.hh"
#include "netstack/header/ipv4.hh"
#include "netstack/net/ipv4/protocol.hh"
#include "netstack/stack/stack.hh"

using netstack::IPv4Address;
using netstack::LinkAddress;
using netstack::header::IPv4Fields;
using netstack::header::IPv4Header;
using netstack::link::FdEndpoint;
using netstack::stack::PacketBuffer;
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
  hdr.SetChecksumField(static_cast<uint16_t>(~hdr.CalculateChecksum()));
  return buf;
}

void TestFdEndpointReadDeliver() {
  int sv[2] = {-1, -1};
  assert(::socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0);

  Stack stack;
  stack.AddNetworkProtocol(std::make_unique<netstack::net::ipv4::Protocol>());

  auto ep = std::make_unique<FdEndpoint>(sv[0], 1500, LinkAddress{});
  auto* raw = ep.get();
  const auto nic_id = stack.CreateNIC(std::move(ep));

  const auto frame = MakeIPv4Packet(20, 99);
  const ssize_t wn = ::write(sv[1], frame.data(), frame.size());
  assert(wn == static_cast<ssize_t>(frame.size()));

  raw->PollOnce();
  assert(stack.GetStats(nic_id).nic.rx_packets == 1);

  ::close(sv[1]);
}

void TestFdEndpointWriteOutbound() {
  int sv[2] = {-1, -1};
  assert(::socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0);

  FdEndpoint ep(sv[0], 1500, LinkAddress{});
  std::vector<uint8_t> payload{1, 2, 3, 4};
  PacketBuffer pkt(std::move(payload));
  assert(!ep.WritePacket(nullptr, nullptr,
                         netstack::header::kIPv4ProtocolNumber, std::move(pkt))
              .has_value());

  std::vector<uint8_t> out(4);
  const ssize_t rn = ::read(sv[1], out.data(), out.size());
  assert(rn == 4);
  assert(out[0] == 1 && out[3] == 4);

  ::close(sv[1]);
}

}  // namespace

int main() {
  TestFdEndpointReadDeliver();
  TestFdEndpointWriteOutbound();
  return 0;
}
