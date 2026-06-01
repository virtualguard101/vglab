/**
 * @file nic.hh
 * @brief NIC：实现 NetworkDispatcher，将链路层入站包交给网络协议。
 *
 * NIC（Network Interface Card）在参考实现中同时承担：
 * - 链路上下文（绑定的 LinkEndpoint）；
 * - 入站 demux：按 NetworkProtocolNumber 选 ipv4/ipv6 等；
 * - 部分统计与地址表（M0 仅实现 demux + 计数）。
 *
 * @see references/tcpip/stack/nic.go DeliverNetworkPacket
 * @see docs/m0.md
 */

#pragma once

#include <memory>
#include <unordered_map>

#include "netstack/stack/network_protocol.hh"
#include "netstack/stack/registration.hh"

namespace netstack::stack {

class Stack;

/**
 * @brief 网络接口卡（对标 stack.NIC）。
 *
 * 实现 NetworkDispatcher，故 LinkEndpoint::Attach 时传入 this。
 */
class NIC : public NetworkDispatcher {
 public:
  explicit NIC(NICID id, std::unique_ptr<LinkEndpoint> link_ep);
  ~NIC() override = default;

  NICID ID() const { return id_; }
  LinkEndpoint& GetLinkEndpoint() { return *link_ep_; }
  const LinkEndpoint& GetLinkEndpoint() const { return *link_ep_; }

  /**
   * @brief 注册网络协议（非拥有指针，生命周期由 Stack 保证）。
   * @param number 如 header::kIPv4ProtocolNumber (0x0800)。
   */
  void RegisterProtocol(NetworkProtocolNumber number,
                        NetworkProtocol* protocol);

  /**
   * @brief 链路层入站回调（核心分发函数）。
   *
   * 调用链：loopback WritePacket / channel InjectInbound → 本函数 →
   * NetworkProtocol::HandlePacket。
   */
  void DeliverNetworkPacket(LinkEndpoint& link_ep, LinkAddress remote,
                            LinkAddress local, NetworkProtocolNumber protocol,
                            PacketBuffer pkt) override;

  uint64_t RxPackets() const { return rx_packets_; }
  uint64_t RxBytes() const { return rx_bytes_; }
  uint64_t UnknownProtocolRcvd() const { return unknown_protocol_rcvd_; }
  uint64_t MalformedRcvd() const { return malformed_rcvd_; }

 private:
  NICID id_{};
  std::unique_ptr<LinkEndpoint> link_ep_{};
  std::unordered_map<NetworkProtocolNumber, NetworkProtocol*> protocols_{};
  uint64_t rx_packets_{0};
  uint64_t rx_bytes_{0};
  uint64_t unknown_protocol_rcvd_{0};
  uint64_t malformed_rcvd_{0};
};

}  // namespace netstack::stack
