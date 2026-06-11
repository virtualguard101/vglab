/**
 * @file connection.hh
 * @brief 单条 TCP 连接的状态机（M2+：被动/主动打开、四元组 demux）。
 *
 * ## 与 Listener 的分工
 *
 * | 组件 | RFC 793 角色 | demuxer 键 |
 * |------|--------------|------------|
 * | `Listener` | LISTEN，只处理 SYN | `(local_ip, local_port)` |
 * | `Connection` | SYN-SENT … CLOSED | **完整四元组** |
 *
 * 被动打开：Listener 收到 SYN 后 `CreatePassive()` 生成本对象并登记四元组。
 * 主动打开：应用调用 `Connect()`，发 SYN 并登记四元组。
 *
 * @see docs/tcp-rfc793-states.md
 * @see docs/m2+.md
 */

#pragma once

#include <cstdint>
#include <vector>

#include "netstack/address.hh"
#include "netstack/seqnum.hh"
#include "netstack/stack/registration.hh"
#include "netstack/stack/transport.hh"
#include "netstack/transport/tcp/state.hh"

namespace netstack::stack {
class Stack;
}

namespace netstack::transport::tcp {

class Listener;

class Connection : public stack::TransportEndpoint {
  friend class Listener;

 public:
  explicit Connection(stack::Stack* stack);

  /**
   * @brief 主动打开（RFC 793 active OPEN）：CLOSED → SYN-SENT。
   *
   * 发送 SYN 后向 demuxer 登记四元组，以便收到 SYN-ACK 时路由到本连接。
   */
  stack::StackResult Connect(stack::NICID nic_id, IPv4Address local_addr,
                             uint16_t local_port, IPv4Address remote_addr,
                             uint16_t remote_port);

  TcpState State() const { return state_; }
  seqnum::Value Iss() const { return iss_; }

  std::vector<uint8_t> Read();
  stack::StackResult Write(std::span<const uint8_t> data);
  void Close();

  void HandlePacket(stack::Route* route, stack::TransportEndpointID id,
                    stack::PacketBuffer pkt) override;

 private:
  friend class Listener;

  /**
   * @brief Listener 在收到 SYN 时调用；进入 SYN-RECEIVED 并发 SYN|ACK。
   *
   * 不占用端口（端口由 Listener Reserve）；登记四元组供后续 ACK/数据 demux。
   */
  void BeginPassiveOpen(stack::NICID nic_id, IPv4Address local_addr,
                        uint16_t local_port, stack::Route* route,
                        stack::TransportEndpointID id, uint32_t client_seq);

  stack::TransportEndpointID EndpointId() const;

  void SendSegment(stack::Route* route, uint8_t flags, uint32_t seq,
                   uint32_t ack, std::span<const uint8_t> payload);

  stack::StackResult RegisterConnected();
  void UnregisterConnected();
  void NotifyEstablished();

  stack::Stack* stack_{nullptr};
  Listener* listener_{nullptr};
  stack::NICID nic_id_{};
  IPv4Address local_addr_{};
  uint16_t local_port_{};
  uint16_t remote_port_{};
  stack::Address remote_addr_{};

  TcpState state_{TcpState::kClosed};
  seqnum::Value iss_{};
  seqnum::Value rcv_nxt_{};
  seqnum::Value snd_nxt_{};
  std::vector<uint8_t> rcv_buf_{};
  bool demux_registered_{false};
  /** @brief true 表示本连接通过 Connect() 占用了本地端口。 */
  bool owns_port_{false};
};

}  // namespace netstack::transport::tcp
