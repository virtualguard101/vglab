/**
 * @file tcp_test.cc
 * @brief TCP 头单元测试（M2）。
 *
 * ## 学习目标
 *
 * - TCP 头固定 20 字节（M2 无选项）；
 * - `DataOffset` 存于第 12 字节高 4 位（单位 4 字节）；
 * - Flags 可组合（SYN|ACK）；
 * - `IsValid` 拒绝声称头长 < 20 的段。
 *
 * @code
 * ctest --test-dir build -R header_tcp
 * @endcode
 *
 * @see include/netstack/header/tcp.hh
 * @see docs/m2.md
 */

#include "netstack/header/tcp.hh"

#include <cassert>
#include <vector>

using netstack::header::TCPFields;
using netstack::header::TCPFlags;
using netstack::header::TCPHeader;

namespace {

void TestEncodeAndParse() {
  std::vector<uint8_t> buf(20, 0);
  TCPHeader hdr(buf);
  TCPFields fields{};
  fields.src_port = 50000;
  fields.dst_port = 80;
  fields.seq_num = 1000;
  fields.ack_num = 2000;
  fields.data_offset = 20;
  fields.flags = static_cast<uint8_t>(TCPFlags::kSyn | TCPFlags::kAck);
  fields.window_size = 65535;
  fields.checksum = 0;
  hdr.Encode(fields);

  uint16_t src = 0;
  uint16_t dst = 0;
  assert(TCPHeader::ParsePorts(buf, src, dst));
  assert(src == 50000);
  assert(dst == 80);
  assert(hdr.SequenceNumber() == 1000);
  assert(hdr.AcknowledgmentNumber() == 2000);
  assert(hdr.DataOffset() == 20);
  assert(hdr.Flags() == static_cast<uint8_t>(TCPFlags::kSyn | TCPFlags::kAck));
  assert(hdr.IsValid(buf.size()));
}

/** @brief data_offset=16 字节 → 非法（小于最小 20）。 */
void TestInvalidDataOffset() {
  std::vector<uint8_t> buf(20, 0);
  TCPHeader hdr(buf);
  TCPFields fields{};
  fields.data_offset = 16;
  fields.flags = static_cast<uint8_t>(TCPFlags::kSyn);
  hdr.Encode(fields);
  assert(!hdr.IsValid(buf.size()));
}

}  // namespace

int main() {
  TestEncodeAndParse();
  TestInvalidDataOffset();
  return 0;
}
