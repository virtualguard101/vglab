/**
 * @file stack.hh
 * @brief 协议栈：NIC、网络/传输协议注册与分发（M0/M1）。
 *
 * ## M1 完整入站路径
 *
 * @code
 * LinkEndpoint → NIC::DeliverNetworkPacket
 *   → ipv4::HandlePacket（填 Route 源/目的 IP）
 *   → Stack::DeliverTransportPacket
 *   → TransportDemuxer → udp::Endpoint
 * @endcode
 *
 * IPv4 注册时通过 `SetTransportDispatcherIfUnset(this)` 接入 demuxer，
 * 不覆盖 M0 测试手动设置的 `RecordingTransportDispatcher`。
 *
 * @see docs/m0.md
 * @see docs/m1.md
 */

#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "netstack/address.hh"
#include "netstack/ports/port_manager.hh"
#include "netstack/stack/network_protocol.hh"
#include "netstack/stack/nic.hh"
#include "netstack/stack/registration.hh"
#include "netstack/stack/transport.hh"

namespace netstack::stack {

struct NICStats {
  uint64_t rx_packets{};
  uint64_t rx_bytes{};
  uint64_t unknown_protocol_rcvd{};
  uint64_t malformed_rcvd{};
};

struct StackStats {
  NICStats nic{};
  uint64_t ipv4_malformed_received{};
  uint64_t ipv4_packets_delivered{};
};

/**
 * @brief 协议栈：实现 TransportDispatcher，拥有 demuxer 与 PortManager。
 */
class Stack : public TransportDispatcher {
 public:
  Stack();

  void AddNetworkProtocol(std::unique_ptr<NetworkProtocol> protocol);
  void AddTransportProtocol(std::unique_ptr<TransportProtocol> protocol);

  NICID CreateNIC(std::unique_ptr<LinkEndpoint> link_ep);

  /** @brief 登记 NIC 上的本机 IPv4（M1 最小；可多次添加）。 */
  StackResult AddAddress(NICID nic_id, IPv4Address addr);

  NIC* GetNIC(NICID id);
  const NIC* GetNIC(NICID id) const;

  NetworkProtocol* FindProtocol(NetworkProtocolNumber number);
  const NetworkProtocol* FindProtocol(NetworkProtocolNumber number) const;

  StackStats GetStats(NICID id) const;

  bool ReservePort(NetworkProtocolNumber net, TransportProtocolNumber trans,
                   uint16_t port);
  void ReleasePort(NetworkProtocolNumber net, TransportProtocolNumber trans,
                   uint16_t port);

  StackResult RegisterTransportEndpoint(NetworkProtocolNumber net,
                                        TransportProtocolNumber trans,
                                        TransportEndpointID id,
                                        TransportEndpoint* endpoint);

  void DeliverTransportPacket(Route* route, TransportProtocolNumber protocol,
                              PacketBuffer pkt) override;

 private:
  std::unordered_map<NetworkProtocolNumber, std::unique_ptr<NetworkProtocol>>
      protocols_{};
  std::unordered_map<NICID, std::unique_ptr<NIC>> nics_{};
  std::unordered_map<NICID, std::vector<IPv4Address>> nic_addresses_{};
  TransportDemuxer demuxer_{};
  ports::PortManager port_manager_{};
  NICID next_nic_id_{1};
};

}  // namespace netstack::stack
