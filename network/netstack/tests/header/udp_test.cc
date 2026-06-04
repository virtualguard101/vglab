/**
 * @file udp_test.cc
 * @brief UDP 头单元测试（M1）。
 */

#include <cassert>
#include <vector>

#include "netstack/header/udp.hh"

using netstack::header::UDPFields;
using netstack::header::UDPHeader;

namespace {

void TestEncodeAndParsePorts() {
  std::vector<uint8_t> buf(8, 0);
  UDPHeader hdr(buf);
  UDPFields fields{};
  fields.src_port = 12345;
  fields.dst_port = 7;
  fields.length = 8;
  fields.checksum = 0;
  hdr.Encode(fields);

  uint16_t src = 0;
  uint16_t dst = 0;
  assert(UDPHeader::ParsePorts(buf, src, dst));
  assert(src == 12345);
  assert(dst == 7);
  assert(hdr.IsValid(buf.size()));
  assert(hdr.Length() == 8);
}

void TestInvalidLength() {
  std::vector<uint8_t> buf(8, 0);
  UDPHeader hdr(buf);
  hdr.SetLength(100);
  assert(!hdr.IsValid(buf.size()));
}

}  // namespace

int main() {
  TestEncodeAndParsePorts();
  TestInvalidLength();
  return 0;
}
