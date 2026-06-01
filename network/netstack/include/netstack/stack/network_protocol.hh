/**
 * @file network_protocol.hh
 * @brief 网络层协议接口（对标 stack.NetworkProtocol / NetworkEndpoint）。
 */

#pragma once

#include <cstddef>

#include "netstack/stack/registration.hh"

namespace netstack::stack {

/**
 * @brief 传输层分发器：网络层将剥头后的包交给传输层（M0 可用 stub）。
 */
class TransportDispatcher {
 public:
  virtual ~TransportDispatcher() = default;

  virtual void DeliverTransportPacket(Route* route,
                                      TransportProtocolNumber protocol,
                                      PacketBuffer pkt) = 0;
};

/**
 * @brief 网络层协议（ipv4/ipv6），由 NIC 在 DeliverNetworkPacket 中调用。
 */
class NetworkProtocol {
 public:
  virtual ~NetworkProtocol() = default;

  virtual NetworkProtocolNumber Number() const = 0;
  virtual int MinimumPacketSize() const = 0;

  /** @brief 从包数据起始处解析源/目的网络地址。*/
  virtual void ParseAddresses(const std::vector<uint8_t>& packet, Address& src,
                              Address& dst) const = 0;

  /**
   * @brief 处理入站网络层包（对标 NetworkEndpoint.HandlePacket）。
   * @param pkt Data 以网络层头开始；实现方会剥离头并可能交付传输层。
   */
  virtual void HandlePacket(Route* route, PacketBuffer pkt) = 0;

  void SetTransportDispatcher(TransportDispatcher* dispatcher) {
    transport_dispatcher_ = dispatcher;
  }

 protected:
  TransportDispatcher* transport_dispatcher_{nullptr};
};

/** @brief 记录 DeliverTransportPacket 调用，供 M0 测试。*/
class RecordingTransportDispatcher : public TransportDispatcher {
 public:
  struct Entry {
    TransportProtocolNumber protocol{};
    size_t payload_size{};
  };

  void DeliverTransportPacket(Route* route, TransportProtocolNumber protocol,
                              PacketBuffer pkt) override;

  const std::vector<Entry>& Entries() const { return entries_; }
  void Clear() { entries_.clear(); }

 private:
  std::vector<Entry> entries_{};
};

}  // namespace netstack::stack
