/**
 * @file network_protocol.hh
 * @brief 网络层协议接口与 M0 传输层测试桩。
 *
 * ## 分层关系
 *
 * - **NetworkProtocol**：IPv4/IPv6 等，处理 L3 头并可能下交传输层；
 * - **TransportDispatcher**：网络层与传输层之间的窄接口（参考实现中由 NIC/Stack
 *   实现更完整的 demux）。
 *
 * 典型入站路径：
 * @code
 * LinkEndpoint → NIC → NetworkProtocol::HandlePacket
 *   → TransportDispatcher::DeliverTransportPacket → TransportDemuxer
 * @endcode
 *
 * ## M0 测试桩 vs M1 生产路径
 *
 * M0 用 `RecordingTransportDispatcher` 验证「IPv4 剥头后是否调用了交付」；
 * M1 起 `Stack` 注入真实 demuxer，按四元组分发到 UDP/TCP Endpoint。
 *
 * @see src/stack/network_protocol.cc
 * @see references/tcpip/stack/registration.go NetworkProtocol
 * @see docs/m0.md
 * @see docs/m1.md
 */

#pragma once

#include <cstddef>

#include "netstack/stack/registration.hh"

namespace netstack::stack {

/**
 * @brief 传输层分发器：网络层将剥头后的包交给传输层。
 *
 * @param route 出站/入站路由上下文（M0 字段较少，M1 填 local/remote IP）。
 * @param protocol IPv4 头中的 Protocol 字段（如 17=UDP）。
 * @param pkt Data 已为传输层载荷（无 IPv4 头）。
 */
class TransportDispatcher {
 public:
  virtual ~TransportDispatcher() = default;

  virtual void DeliverTransportPacket(Route* route,
                                      TransportProtocolNumber protocol,
                                      PacketBuffer pkt) = 0;
};

/**
 * @brief 网络层协议抽象（ipv4、ipv6 等实现本接口）。
 */
class NetworkProtocol {
 public:
  virtual ~NetworkProtocol() = default;

  virtual NetworkProtocolNumber Number() const = 0;

  /** @brief NIC 在调用 HandlePacket 前用于丢弃过短包。 */
  virtual int MinimumPacketSize() const = 0;

  /** @brief 从仍以 L3 头开头的 packet 解析源/目的地址。 */
  virtual void ParseAddresses(const std::vector<uint8_t>& packet, Address& src,
                              Address& dst) const = 0;

  /**
   * @brief 处理入站网络层包。
   *
   * @param route 可为 nullptr；非空时实现应填写 local/remote 地址（入站语义）。
   * @param pkt 入参 Data 以网络层头开始；实现通常剥头并
   * DeliverTransportPacket。
   */
  virtual void HandlePacket(Route* route, PacketBuffer pkt) = 0;

  /** @brief 由 Stack 或测试在注册后注入（非拥有指针）。 */
  void SetTransportDispatcher(TransportDispatcher* dispatcher) {
    transport_dispatcher_ = dispatcher;
  }

  /** @brief 仅在尚未设置时绑定 dispatcher（避免覆盖测试桩）。 */
  void SetTransportDispatcherIfUnset(TransportDispatcher* dispatcher) {
    if (transport_dispatcher_ == nullptr) {
      transport_dispatcher_ = dispatcher;
    }
  }

 protected:
  TransportDispatcher* transport_dispatcher_{nullptr};
};

/**
 * @brief 记录 DeliverTransportPacket 调用，供 M0/M1 集成测试断言。
 *
 * 每条 Entry 保存上层协议号与载荷长度；不保存 payload 内容以简化测试。
 */
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
