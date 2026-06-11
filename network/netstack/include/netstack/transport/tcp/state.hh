/**
 * @file state.hh
 * @brief TCP 连接状态枚举（M2 子集 ↔ RFC 793 Figure 6）。
 *
 * ## RFC 793 状态图（M2 高亮路径）
 *
 * 完整 Mermaid / 转移表见 **docs/tcp-rfc793-states.md**。
 *
 * @code
 *                    ┌───────────┐
 *         Listen()   │   CLOSED   │◄── RCV ACK (LAST-ACK) [M2]
 *            [M2]    └─────┬─────┘
 *                    ┌─────▼─────┐
 *                    │   LISTEN   │  kListen
 *                    └─────┬─────┘
 *              RCV SYN [M2]│
 *                    ┌─────▼──────────┐
 *                    │ SYN-RECEIVED   │  kSynReceived
 *                    └─────┬──────────┘
 *              RCV ACK [M2]│
 *                    ┌─────▼──────────┐
 *         ┌──────────│  ESTABLISHED   │──────── Close() [M2]
 *         │          └─────┬──────────┘
 *  FIN-WAIT-1 [—]         │ RCV FIN [M2]
 *         │          ┌─────▼──────────┐
 *         │          │  CLOSE-WAIT    │  kCloseWait
 *         │          └─────┬──────────┘
 *         │         Close() [M2]
 *         │          ┌─────▼──────────┐
 *         │          │   LAST-ACK     │  kLastAck
 *         │          └────────────────┘
 *         └─ (主动关闭完整路径，M2+)
 * @endcode
 *
 * | `TcpState` | RFC 793 | M2 |
 * |------------|---------|-----|
 * | kClosed | CLOSED | ✓ |
 * | kListen | LISTEN | ✓ |
 * | kSynReceived | SYN-RECEIVED | ✓ |
 * | kEstablished | ESTABLISHED | ✓ |
 * | kCloseWait | CLOSE-WAIT | ✓ |
 * | kLastAck | LAST-ACK | ✓ |
 * | （无） | SYN-SENT, FIN-WAIT-*, CLOSING, TIME-WAIT | M2+ |
 *
 * @see docs/tcp-rfc793-states.md
 * @see RFC 793 Figure 6
 * @see docs/adr/004-m2-tcp-simplified.md
 */

#pragma once

namespace netstack::transport::tcp {

enum class TcpState {
  kClosed,       ///< RFC: CLOSED — 无连接 / 已终止
  kListen,       ///< RFC: LISTEN — passive OPEN 后等待 SYN
  kSynReceived,  ///< RFC: SYN-RECEIVED — 已发 SYN-ACK，等第三次 ACK
  kEstablished,  ///< RFC: ESTABLISHED — 数据传送态
  kCloseWait,    ///< RFC: CLOSE-WAIT — 对端 FIN 已收，等本端 CLOSE
  kLastAck,      ///< RFC: LAST-ACK — 本端 FIN 已发，等对端 ACK
};

}  // namespace netstack::transport::tcp
