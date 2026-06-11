/**
 * @file seqnum.hh
 * @brief TCP 序列号算术（对标 references/tcpip/seqnum/seqnum.go）。
 *
 * 使用 int32 差值比较处理 32 位回绕。
 *
 * @see docs/m2.md
 */

#pragma once

#include <cstdint>

namespace netstack::seqnum {

using Value = uint32_t;
using Size = uint32_t;

inline bool LessThan(Value v, Value w) {
  return static_cast<int32_t>(v - w) < 0;
}

inline bool LessThanEq(Value v, Value w) {
  return v == w || LessThan(v, w);
}

inline Value Add(Value v, Size s) { return v + s; }

inline bool InRange(Value v, Value a, Value b) { return (v - a) < (b - a); }

inline bool InWindow(Value v, Value first, Size size) {
  return InRange(v, first, Add(first, size));
}

inline Size SizeBetween(Value v, Value w) { return static_cast<Size>(w - v); }

}  // namespace netstack::seqnum
