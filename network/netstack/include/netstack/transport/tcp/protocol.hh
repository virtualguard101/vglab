/**
 * @file protocol.hh
 * @brief TCP 传输层协议注册对象（M2）。
 *
 * ## 职责边界
 *
 * 实现 `TransportProtocol` 的**元数据**接口，供 demuxer 在入站时：
 * - 确认最小段长 ≥ 20 字节（无选项 TCP 头）；
 * - 用 `ParsePorts` 读出源/目的端口，与 Route 中 IP 组成四元组键。
 *
 * 不持有连接状态；每个 `Endpoint` 单独 `Listen`。状态机在
 * `tcp::Connection` / `tcp::Listener` 中按 **RFC 793** 分支。
 *
 * ## 与 UDP Protocol 的对比
 *
 * | | UDP | TCP |
 * |---|-----|-----|
 * | 协议号 | 17 | 6 |
 * | 最小头 | 8 | 20 |
 * | 状态 | 无连接，Endpoint 直接处理 | 四元组 → Connection |
 *
 * @see src/transport/tcp/protocol.cc
 * @see references/tcpip/transport/tcp/protocol.go
 * @see docs/tcp-rfc793-states.md
 * @see docs/m2.md
 */

#pragma once

#include "netstack/stack/transport.hh"

namespace netstack::transport::tcp {

class Protocol : public stack::TransportProtocol {
 public:
  /** @return IPv4 协议号 6。 */
  stack::TransportProtocolNumber Number() const override;

  /** @return TCP 头最小长度 20。 */
  int MinimumPacketSize() const override;

  /**
   * @brief 从 TCP 头解析源/目的端口（选项字段之前的前 4 字节）。
   * @return false 若 transport_hdr 短于 20 字节。
   */
  bool ParsePorts(std::span<const uint8_t> transport_hdr, uint16_t& src,
                  uint16_t& dst) const override;
};

}  // namespace netstack::transport::tcp
