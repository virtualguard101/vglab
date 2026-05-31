/**
 * @file ipv4_test.cc
 * @brief header 模块单元测试（M0：子网默认值 + IPv4 头编码/校验）。
 *
 * 运行：构建后执行 netstack_header_test，或 `ctest --test-dir build`。
 */

#include "netstack/header/ipv4.hh"

#include <cassert>
#include <cstdint>
#include <vector>

#include "netstack/subnet.hh"

using netstack::IPv4Address;
using netstack::kIPv4EmptySubnet;
using netstack::Subnet;
using netstack::header::IPv4Fields;
using netstack::header::IPv4Header;

namespace {

/** @brief 验证 0.0.0.0/0 与 Subnet::Contains 语义。 */
void TestEmptySubnet() {
  const auto subnet = Subnet::New(IPv4Address{{0, 0, 0, 0}}, 0);
  assert(subnet.has_value());
  assert(subnet->prefix_length == 0);
  assert(subnet->Contains(IPv4Address{{10, 0, 0, 1}}));
  assert(kIPv4EmptySubnet.prefix_length == 0);
}

/** @brief 构造最小 IPv4 头，填写校验和并断言 IsValid / IsChecksumValid。 */
void TestEncodeAndValidate() {
  std::vector<uint8_t> buf(20, 0);
  IPv4Header hdr(buf);

  IPv4Fields fields{};
  fields.ihl = 20;
  fields.total_length = 20;
  fields.ttl = 64;
  fields.protocol = 17;  // UDP
  fields.src_addr = IPv4Address{{10, 0, 0, 1}};
  fields.dst_addr = IPv4Address{{10, 0, 0, 2}};
  hdr.Encode(fields);

  hdr.SetChecksumField(0);
  const uint16_t csum = ~hdr.CalculateChecksum();
  hdr.SetChecksumField(csum);

  assert(hdr.IsValid(static_cast<int>(buf.size())));
  assert(hdr.IsChecksumValid());
  assert(hdr.SourceAddress() == fields.src_addr);
  assert(hdr.DestinationAddress() == fields.dst_addr);
}

}  // namespace

int main() {
  TestEmptySubnet();
  TestEncodeAndValidate();
  return 0;
}
