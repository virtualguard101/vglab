/**
 * @file endpoint.hh
 * @brief TCP endpoint：Listen、握手、按序收发包、FIN 关闭（M2）。
 *
 * ## 角色
 *
 * 教学栈中的「TCP socket」极简抽象：单线程、无阻塞 waiter，测试通过
 * `Read()` / `State()` 轮询。对标 references/tcpip/transport/tcp/endpoint.go
 * 的大幅裁剪版。
 *
 * ## RFC 793 被动打开（M2 主路径）
 *
 * 对应状态图：CLOSED → LISTEN → SYN-RECEIVED → ESTABLISHED
 * （见 docs/tcp-rfc793-states.md §「段与状态转移」）
 *
 * @code
 * Listen(80)                    // CLOSED → LISTEN
 *   ← SYN (client)              // LISTEN → SYN-RECEIVED，发 SYN|ACK
 *   → SYN-ACK (seq=iss_, ack=client_seq+1)
 *   ← ACK                       // SYN-RECEIVED → ESTABLISHED
 * @endcode
 *
 * ## RFC 793 关闭（M2 被动关 / 主动关）
 *
 * @code
 * 对端先关：ESTABLISHED --RCV FIN--> CLOSE-WAIT --Close()--> LAST-ACK --RCV
 * ACK--> CLOSED 本端先关：ESTABLISHED --Close()--> LAST-ACK --RCV ACK--> CLOSED
 *           （跳过 FIN-WAIT-1/2、TIME-WAIT，见 ADR 004）
 * @endcode
 *
 * ## 关键序列号字段
 *
 * | 字段 | 含义 |
 * |------|------|
 * | `iss_` | 本端初始序列号（M2 固定 100000，便于测试断言） |
 * | `rcv_nxt_` | 期望收到的下一个字节序号（按序交付） |
 * | `snd_nxt_` | 下一个待发送字节的序号 |
 *
 * ## 入站路径
 *
 * `pkt.Data()` = [TCP 头 | payload]（IPv4 已由 `ipv4::HandlePacket` 剥掉）。
 *
 * @see docs/tcp-rfc793-states.md
 * @see docs/m2.md
 * @see docs/adr/004-m2-tcp-simplified.md
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

class Endpoint : public stack::TransportEndpoint {
 public:
  explicit Endpoint(stack::Stack* stack);

  /**
   * @brief 在本地地址/端口上进入 LISTEN，并注册到 demuxer。
   *
   * RFC 793：**passive OPEN**，CLOSED → LISTEN。
   * 流程与 UDP `Bind` 相同：ReservePort → RegisterTransportEndpoint。
   */
  stack::StackResult Listen(stack::NICID nic_id, IPv4Address local_addr,
                            uint16_t port);

  TcpState State() const { return state_; }

  /** @brief 本端初始序列号（测试用来断言 SYN-ACK 的 seq）。 */
  seqnum::Value Iss() const { return iss_; }

  /** @brief 取出并清空接收缓冲区（M2 单段、同步模型）。 */
  std::vector<uint8_t> Read();

  /** @brief 在 ESTABLISHED 态发送一个 PSH+ACK 段。 */
  stack::StackResult Write(std::span<const uint8_t> data);

  /**
   * @brief 发送 FIN 并进入 LAST_ACK。
   *
   * RFC 793：ESTABLISHED/CLOSE-WAIT 上 **CLOSE** → 发 FIN|ACK → LAST-ACK。
   */
  void Close();

  void HandlePacket(stack::Route* route, stack::TransportEndpointID id,
                    stack::PacketBuffer pkt) override;

 private:
  /** @brief 组装 TCP 段 + 伪首部校验和 + `ipv4::SendPacket`。 */
  void SendSegment(stack::Route* route, uint8_t flags, uint32_t seq,
                   uint32_t ack, std::span<const uint8_t> payload);

  stack::Stack* stack_{nullptr};
  stack::NICID nic_id_{};
  IPv4Address local_addr_{};
  uint16_t local_port_{};
  uint16_t remote_port_{};
  stack::Address remote_addr_{};

  TcpState state_{TcpState::kClosed};
  seqnum::Value iss_{100000};
  seqnum::Value rcv_nxt_{};
  seqnum::Value snd_nxt_{};
  std::vector<uint8_t> rcv_buf_{};
};

}  // namespace netstack::transport::tcp
