/**
 * @file transport.hh
 * @brief 传输层接口与 demuxer（M1 UDP / M2 TCP / M2+ 四元组）。
 *
 * ## 分层位置
 *
 * IPv4 `HandlePacket` 剥掉 L3 头后，由 `Stack::DeliverTransportPacket` 进入
 * **TransportDemuxer**，再按 **本地端口 + 本地 IP** 或 **四元组** 找到端点。
 *
 * @code
 * ipv4::HandlePacket
 *   → Stack::DeliverTransportPacket(proto=17|6, pkt 含 UDP/TCP 头)
 *   → TransportDemuxer::DeliverPacket
 *   → udp::Endpoint / tcp::Listener / tcp::Connection
 * @endcode
 *
 * ## TCP demuxer 双表（M2+）
 *
 * | 表 | 键 | 端点类型 | 典型状态 |
 * |----|-----|----------|----------|
 * | `listen_endpoints_` | (local_ip, local_port) | `tcp::Listener` | LISTEN |
 * | `connected_endpoints_` | 四元组 | `tcp::Connection` | SYN-SENT … CLOSED |
 *
 * 查找顺序：**先四元组，再监听键**（仅 TCP）。
 *
 * @see docs/tcp-rfc793-states.md
 * @see docs/m2+.md
 */

#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <unordered_map>

#include "netstack/stack/registration.hh"

namespace netstack::stack {

struct TransportEndpointID {
  uint16_t local_port{};
  Address local_address;
  uint16_t remote_port{};
  Address remote_address;
};

class TransportEndpoint {
 public:
  virtual ~TransportEndpoint() = default;
  virtual void HandlePacket(Route* route, TransportEndpointID id,
                            PacketBuffer pkt) = 0;
};

class TransportProtocol {
 public:
  virtual ~TransportProtocol() = default;

  virtual TransportProtocolNumber Number() const = 0;
  virtual int MinimumPacketSize() const = 0;

  virtual bool ParsePorts(std::span<const uint8_t> transport_hdr, uint16_t& src,
                          uint16_t& dst) const = 0;

  virtual void HandleUnknownDestinationPacket(Route* route,
                                              TransportEndpointID id,
                                              PacketBuffer pkt) {
    (void)route;
    (void)id;
    (void)pkt;
  }
};

class TransportDemuxer {
 public:
  void AddProtocol(std::unique_ptr<TransportProtocol> protocol);

  /** @brief UDP Bind / TCP Listen 登记（本地 IP + 端口）。 */
  StackResult RegisterEndpoint(NetworkProtocolNumber net_proto,
                               TransportProtocolNumber trans_proto,
                               TransportEndpointID id,
                               TransportEndpoint* endpoint);

  /**
   * @brief TCP 已建立（或半连接）四元组登记。
   *
   * id 须含 local_* 与 remote_*；与 `RegisterEndpoint` 监听键互不冲突。
   */
  StackResult RegisterConnectedEndpoint(NetworkProtocolNumber net_proto,
                                        TransportProtocolNumber trans_proto,
                                        TransportEndpointID id,
                                        TransportEndpoint* endpoint);

  void UnregisterConnectedEndpoint(NetworkProtocolNumber net_proto,
                                   TransportProtocolNumber trans_proto,
                                   TransportEndpointID id);

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

  struct TupleKey {
    NetworkProtocolNumber net{};
    TransportProtocolNumber trans{};
    uint16_t local_port{};
    Address local_address;
    uint16_t remote_port{};
    Address remote_address;

    bool operator==(const TupleKey& o) const {
      return net == o.net && trans == o.trans && local_port == o.local_port &&
             local_address == o.local_address && remote_port == o.remote_port &&
             remote_address == o.remote_address;
    }
  };

  struct BindKeyHash {
    size_t operator()(const BindKey& k) const;
  };

  struct TupleKeyHash {
    size_t operator()(const TupleKey& k) const;
  };

  static TupleKey ToTupleKey(NetworkProtocolNumber net,
                             TransportProtocolNumber trans,
                             const TransportEndpointID& id);

  std::unordered_map<BindKey, TransportEndpoint*, BindKeyHash>
      listen_endpoints_{};
  std::unordered_map<TupleKey, TransportEndpoint*, TupleKeyHash>
      connected_endpoints_{};
  std::unordered_map<TransportProtocolNumber,
                     std::unique_ptr<TransportProtocol>>
      protocols_{};
};

}  // namespace netstack::stack
