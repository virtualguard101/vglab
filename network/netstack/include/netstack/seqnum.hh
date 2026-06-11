/**
 * @file seqnum.hh
 * @brief TCP 序列号算术（对标 references/tcpip/seqnum/seqnum.go）。
 *
 * ## 为什么需要专用算术？
 *
 * TCP 序列号是 **32 位无符号整数**，会周期性回绕（0xFFFFFFFF → 0）。
 * 若直接用 `v < w` 比较，回绕附近会得出错误结论。
 *
 * 本模块用 **有符号差值** 判断先后关系，与 Go 参考实现一致：
 *
 * @code
 * LessThan(v, w)  ⇔  int32(v - w) < 0
 * @endcode
 *
 * 在「半环」语义下，这等价于：在不超过 2^31 的窗口内，v 是否在 w 之前到达。
 *
 * ## 教学示例
 *
 * | 表达式 | 结果 | 含义 |
 * |--------|------|------|
 * | `LessThan(0xFFFFFFFF, 0)` | true | 回绕后 0 在 MAX 之后 |
 * | `LessThan(100, 200)` | true | 正常递增 |
 * | `InWindow(120, 100, 50)` | true | 120 ∈ [100, 150) |
 *
 * ## 与 RFC 793 状态转移中的序号（§3.3）
 *
 * | 事件 | 序号变化 | 代码字段 |
 * |------|----------|----------|
 * | 收到 SYN seq=S | 下一期望字节 = S+1 | `rcv_nxt_ = Add(S,1)` |
 * | 发 SYN seq=ISS | 下一发送字节 = ISS+1 | `snd_nxt_ = Add(ISS,1)` |
 * | 收到 len=L 数据 | `rcv_nxt_ += L` | `HandlePacket` ESTABLISHED |
 * | 收到 FIN | `rcv_nxt_ += 1` | FIN 占 1 序号 |
 * | 发 FIN | `snd_nxt_ += 1` | `Close()` |
 *
 * 比较 seq 与 `rcv_nxt_` 时必须用 `LessThan` / 相等判断，禁止 `seq <
 * rcv_nxt_`。
 *
 * @see docs/tcp-rfc793-states.md §「序列号在转移中的变化」
 * @see docs/m2.md
 * @see references/tcpip/seqnum/seqnum.go
 */

#pragma once

#include <cstdint>

namespace netstack::seqnum {

/** @brief 32 位 TCP 序列号（RFC 793）。 */
using Value = uint32_t;

/** @brief 序列号上的「长度」或增量（字节数、窗口大小等）。 */
using Size = uint32_t;

/**
 * @brief 判断 v 是否在 w 之前（回绕安全）。
 *
 * 实现：`static_cast<int32_t>(v - w) < 0`。
 * 不要用 `v < w` 替代。
 */
inline bool LessThan(Value v, Value w) {
  return static_cast<int32_t>(v - w) < 0;
}

/** @brief v ≤ w（相等或 LessThan）。 */
inline bool LessThanEq(Value v, Value w) { return v == w || LessThan(v, w); }

/** @brief 序列号加法（uint32 自然回绕）。 */
inline Value Add(Value v, Size s) { return v + s; }

/**
 * @brief 判断 v 是否落在半开区间 [a, b) 内（按无符号差值）。
 *
 * 用于检查某个 seq 是否落在对端已发送但未确认的范围。
 */
inline bool InRange(Value v, Value a, Value b) { return (v - a) < (b - a); }

/**
 * @brief 判断 v 是否落在接收窗口 [first, first+size) 内。
 *
 * M2 固定窗口 64KiB；乱序段若不在窗口内应丢弃。
 */
inline bool InWindow(Value v, Value first, Size size) {
  return InRange(v, first, Add(first, size));
}

/** @brief 计算 w - v 的字节距离（假定 w ≥ v 且未跨半环）。 */
inline Size SizeBetween(Value v, Value w) { return static_cast<Size>(w - v); }

}  // namespace netstack::seqnum
