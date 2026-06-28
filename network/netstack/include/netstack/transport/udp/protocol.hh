/**
 * @file protocol.hh
 * @brief UDP 传输层协议注册对象（M1）。
 *
 * ## 职责边界
 *
 * 实现 `TransportProtocol` 的**元数据**接口，供 `TransportDemuxer` 入站分发：
 * - 协议号 17；
 * - 最小头长 8 字节；
 * - `ParsePorts` 从 L4 头提取源/目的端口。
 *
 * 不持有 socket 状态；每个 `Endpoint` 单独 `Bind`，echo 逻辑在
 * `udp::Endpoint::HandlePacket`。
 *
 * @see src/transport/udp/protocol.cc
 * @see references/tcpip/transport/udp/protocol.go
 * @see docs/m1.md
 */

#pragma once

#include "netstack/stack/transport.hh"

namespace netstack::transport::udp {

class Protocol : public stack::TransportProtocol {
 public:
  /** @return IPv4 协议号 17（UDP）。 */
  stack::TransportProtocolNumber Number() const override;

  /** @return UDP 头最小长度 8。 */
  int MinimumPacketSize() const override;

  /**
   * @brief 从传输层头前 8 字节解析源/目的端口。
   * @return false 若 transport_hdr 短于最小 UDP 头。
   */
  bool ParsePorts(std::span<const uint8_t> transport_hdr, uint16_t& src,
                  uint16_t& dst) const override;
};

}  // namespace netstack::transport::udp
