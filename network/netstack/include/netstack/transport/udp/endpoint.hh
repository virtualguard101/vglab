/**
 * @file endpoint.hh
 * @brief UDP endpoint：Bind + echo 回射（M1）。
 *
 * ## Echo 语义
 *
 * 收到发往 `(local_addr, port)` 的 UDP 报文后，将 **载荷原样** 回给
 * 发送方：交换 IP（Route 中 local/remote）与 UDP 端口（本机端口 ↔ 远端端口）。
 *
 * 出站经 `net::ipv4::SendPacket` → `LinkEndpoint::WritePacket`；
 * 在 channel 测试中可从 `DrainOutbound()` 读到回显帧。
 *
 * @see docs/m1.md
 * @see docs/adr/003-m1-udp-echo.md
 */

#pragma once

#include <cstdint>

#include "netstack/address.hh"
#include "netstack/stack/registration.hh"
#include "netstack/stack/transport.hh"

namespace netstack::stack {
class Stack;
}

namespace netstack::transport::udp {

class Endpoint : public stack::TransportEndpoint {
 public:
  explicit Endpoint(stack::Stack* stack);

  /**
   * @brief 绑定本地地址与端口，并注册到 demuxer。
   * @param nic_id 回射出站使用的 NIC（channel 测试中与 Inject 相同）。
   */
  stack::StackResult Bind(stack::NICID nic_id, IPv4Address local_addr,
                          uint16_t port);

  void HandlePacket(stack::Route* route, stack::TransportEndpointID id,
                    stack::PacketBuffer pkt) override;

 private:
  stack::Stack* stack_{nullptr};
  stack::NICID nic_id_{};
  IPv4Address local_addr_{};
  uint16_t port_{};
  bool bound_{false};
};

}  // namespace netstack::transport::udp
