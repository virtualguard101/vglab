/**
 * @file listener.hh
 * @brief TCP 监听套接字（M2+ backlog）：常驻 LISTEN，SYN 时 fork Connection。
 *
 * ## RFC 793 与 demuxer
 *
 * - Listener **始终**处于 LISTEN，登记 `(local_ip, local_port)` 监听键。
 * - 每个入站 SYN 创建一条 `Connection`（半连接 → 四元组表）。
 * - 握手完成后 `Accept()` 把 ESTABLISHED 连接交给应用。
 *
 * ## backlog（教学简化）
 *
 * `backlog_` 限制**未完成 + 已就绪**连接总数；超出时丢弃新 SYN（不发 RST）。
 *
 * @see docs/tcp-rfc793-states.md
 * @see docs/m2+.md
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <vector>

#include "netstack/address.hh"
#include "netstack/stack/registration.hh"
#include "netstack/stack/transport.hh"
#include "netstack/transport/tcp/connection.hh"

namespace netstack::stack {
class Stack;
}

namespace netstack::transport::tcp {

class Listener : public stack::TransportEndpoint {
 public:
  explicit Listener(stack::Stack* stack);

  /**
   * @brief passive OPEN：CLOSED → LISTEN，登记监听键。
   * @param backlog 最大并发半连接 + 待 Accept 连接数（默认 8）。
   */
  stack::StackResult Listen(stack::NICID nic_id, IPv4Address local_addr,
                            uint16_t port, size_t backlog = 8);

  /** @brief 取一条已 ESTABLISHED 且尚未被取走的连接；无则 nullptr。 */
  Connection* Accept();

  void HandlePacket(stack::Route* route, stack::TransportEndpointID id,
                    stack::PacketBuffer pkt) override;

 private:
  friend class Connection;

  void EnqueueEstablished(Connection* conn);

  stack::Stack* stack_{nullptr};
  stack::NICID nic_id_{};
  IPv4Address local_addr_{};
  uint16_t local_port_{};
  size_t backlog_{8};
  bool listening_{false};

  std::vector<std::unique_ptr<Connection>> connections_{};
  std::deque<Connection*> accept_queue_{};
};

}  // namespace netstack::transport::tcp
