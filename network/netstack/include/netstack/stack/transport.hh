/**
 * @file transport.hh
 * @brief 传输层接口与 demuxer（M1）。
 *
 * @see references/tcpip/stack/registration.go TransportEndpointID
 * @see references/tcpip/stack/transport_demuxer.go
 * @see docs/m1.md
 */

#pragma once

#include <cstdint>
#include <span>
#include <memory>
#include <unordered_map>
#include <vector>

#include "netstack/stack/registration.hh"

namespace netstack::stack {

/**
 * @brief 传输层端点四元组标识（对标 TransportEndpointID）。
 */
struct TransportEndpointID {
  uint16_t local_port{};
  Address local_address;
  uint16_t remote_port{};
  Address remote_address;
};

/** @brief 可接收报文的传输层端点。 */
class TransportEndpoint {
 public:
  virtual ~TransportEndpoint() = default;
  virtual void HandlePacket(Route* route, TransportEndpointID id,
                            PacketBuffer pkt) = 0;
};

/** @brief 传输层协议（UDP/TCP）抽象。 */
class TransportProtocol {
 public:
  virtual ~TransportProtocol() = default;

  virtual TransportProtocolNumber Number() const = 0;
  virtual int MinimumPacketSize() const = 0;

  virtual bool ParsePorts(std::span<const uint8_t> transport_hdr,
                          uint16_t& src, uint16_t& dst) const = 0;

  virtual void HandleUnknownDestinationPacket(Route* route,
                                            TransportEndpointID id,
                                            PacketBuffer pkt) {
    (void)route;
    (void)id;
    (void)pkt;
  }
};

/**
 * @brief 按 (网络协议, 传输协议, 本地端口, 本地地址) 分发入站包。
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
