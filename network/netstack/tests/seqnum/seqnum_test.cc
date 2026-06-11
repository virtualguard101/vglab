/**
 * @file seqnum_test.cc
 * @brief seqnum 算术单测（M2）。
 *
 * ## 学习目标
 *
 * 1. 理解为何 `0xFFFFFFFF < 0` 在 TCP 语义下为真（回绕）；
 * 2. 验证 `InWindow` 半开区间 [first, first+size)；
 * 3. `Add` 在 uint32 上的自然溢出。
 *
 * @code
 * ctest --test-dir build -R seqnum
 * @endcode
 *
 * @see include/netstack/seqnum.hh
 * @see docs/m2.md
 */

#include "netstack/seqnum.hh"

#include <cassert>

using netstack::seqnum::Add;
using netstack::seqnum::InWindow;
using netstack::seqnum::LessThan;
using netstack::seqnum::Value;

namespace {

/** @brief 回绕比较：MAX 在 0 之前。 */
void TestLessThanWrap() {
  assert(LessThan(0xFFFFFFFFu, 0u));
  assert(!LessThan(0u, 0xFFFFFFFFu));
  assert(LessThan(100u, 200u));
  assert(!LessThan(200u, 100u));
}

/** @brief uint32 加法回绕。 */
void TestAddWrap() {
  assert(Add(0xFFFFFFFFu, 1u) == 0u);
  assert(Add(100u, 50u) == 150u);
}

/** @brief 窗口 [100, 150) 内外边界。 */
void TestInWindow() {
  const Value first = 100u;
  const auto size = 50u;
  assert(InWindow(120u, first, size));
  assert(!InWindow(200u, first, size));
  assert(InWindow(first, first, size));
}

}  // namespace

int main() {
  TestLessThanWrap();
  TestAddWrap();
  TestInWindow();
  return 0;
}
