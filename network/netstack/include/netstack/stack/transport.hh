/**
 * @file transport.hh
 * @brief 传输层接口与 demuxer（M1）。
 *
 * ## 分层位置
 *
 * IPv4 `HandlePacket` 剥掉 L3 头后，由 `Stack::DeliverTransportPacket` 进入
 * **TransportDemuxer**，再按四元组中的 **本地端口 + 本地 IP** 找到
 * `TransportEndpoint`（如 UDP echo socket）。
 *
 * @code
 * ipv4::HandlePacket
 *   → Stack::DeliverTransportPacket(proto=17, pkt 含 UDP 头)
 *   → TransportDemuxer::DeliverPacket
 *   → udp::Endpoint::HandlePacket
 * @endcode
 *
 * @see references/tcpip/stack/registration.go TransportEndpointID
 * @see references/tcpip/stack/transport_demuxer.go
 * @see docs/m1.md
 */

#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <unordered_map>

#include "netstack/stack/registration.hh"

namespace netstack::stack {

/**
 * @brief 传输层端点四元组（对标 TransportEndpointID）。
 *
 * 入站 demux 时：
 * - local_* 来自「发给本机」的一侧（UDP 目的端口 + Route.local_address）；
 * - remote_* 来自发送方（UDP 源端口 + Route.remote_address）。
 */
struct TransportEndpointID {
  uint16_t local_port{};
  Address local_address;
  uint16_t remote_port{};
  Address remote_address;
};

/**
 * @brief 可接收报文的传输层端点（socket 的极简抽象）。
 *
 * 实现类在 `HandlePacket` 内处理数据；M1 的 UDP echo 在此同步回射。
 */
class TransportEndpoint {
 public:
  virtual ~TransportEndpoint() = default;
  virtual void HandlePacket(Route* route, TransportEndpointID id,
                            PacketBuffer pkt) = 0;
};

/**
 * @brief 传输层协议（UDP/TCP）在栈内的注册接口。
 */
class TransportProtocol {
 public:
  virtual ~TransportProtocol() = default;

  virtual TransportProtocolNumber Number() const = 0;
  virtual int MinimumPacketSize() const = 0;

  /** @brief 从仍以传输层头开头的 buffer 解析源/目的端口。 */
  virtual bool ParsePorts(std::span<const uint8_t> transport_hdr, uint16_t& src,
                          uint16_t& dst) const = 0;

  /** @brief 无 listener 时调用（M1 可空实现，不发 ICMP）。 */
  virtual void HandleUnknownDestinationPacket(Route* route,
                                              TransportEndpointID id,
                                              PacketBuffer pkt) {
    (void)route;
    (void)id;
    (void)pkt;
  }
};

/**
 * @brief 传输层分发器（M1 精简版 transportDemuxer）。
 *
 * - `AddProtocol`：注册 UDP 等协议实现（ParsePorts、MinimumPacketSize）；
 * - `RegisterEndpoint`：Bind 时登记 (net, trans, local_port, local_addr)；
 * - `DeliverPacket`：解析端口 → 查找 endpoint → HandlePacket。
 */
class TransportDemuxer {
 public:
  void AddProtocol(std::unique_ptr<TransportProtocol> protocol);

  StackResult RegisterEndpoint(NetworkProtocolNumber net_proto,
                               TransportProtocolNumber trans_proto,
                               TransportEndpointID id,
                               TransportEndpoint* endpoint);

  bool DeliverPacket(Route* route, TransportProtocolNumber trans_proto,
                     PacketBuffer pkt);

  TransportProtocol* FindProtocol(TransportProtocolNumber number);
  const TransportProtocol* FindProtocol(TransportProtocolNumber number) const;

 private:
  struct BindKey {
    NetworkProtocolNumber net{};
    TransportProtocolNumber trans{};
    uint16_t port{};
    Address address;

    bool operator==(const BindKey& o) const {
      return net == o.net && trans == o.trans && port == o.port &&
             address == o.address;
    }
  };

  struct BindKeyHash {
    size_t operator()(const BindKey& k) const;
  };

  std::unordered_map<BindKey, TransportEndpoint*, BindKeyHash> endpoints_{};
  std::unordered_map<TransportProtocolNumber,
                     std::unique_ptr<TransportProtocol>>
      protocols_{};
};

}  // namespace netstack::stack
