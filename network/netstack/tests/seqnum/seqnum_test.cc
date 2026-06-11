/**
 * @file seqnum_test.cc
 * @brief seqnum 算术单测（M2）。
 */

#include "netstack/seqnum.hh"

#include <cassert>

using netstack::seqnum::Add;
using netstack::seqnum::InWindow;
using netstack::seqnum::LessThan;
using netstack::seqnum::Value;

namespace {

void TestLessThanWrap() {
  assert(LessThan(0xFFFFFFFFu, 0u));
  assert(!LessThan(0u, 0xFFFFFFFFu));
  assert(LessThan(100u, 200u));
  assert(!LessThan(200u, 100u));
}

void TestAddWrap() {
  assert(Add(0xFFFFFFFFu, 1u) == 0u);
  assert(Add(100u, 50u) == 150u);
}

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
