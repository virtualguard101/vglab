/**
 * @file protocol.hh
 * @brief TCP 传输层协议注册对象（M2）。
 *
 * 实现 `TransportProtocol`，供 demuxer 在入站时：
 * - 确认最小段长 ≥ 20 字节；
 * - 用 `ParsePorts` 读出源/目的端口以构造四元组键。
 *
 * 不持有连接状态；每个 `Endpoint` 单独 `Listen`。
 *
 * Demuxer 解析端口后把段交给 `Endpoint::HandlePacket`，由后者按
 * **RFC 793 状态图** 分支（见 docs/tcp-rfc793-states.md）。
 *
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

  bool ParsePorts(std::span<const uint8_t> transport_hdr, uint16_t& src,
                  uint16_t& dst) const override;
};

}  // namespace netstack::transport::tcp
