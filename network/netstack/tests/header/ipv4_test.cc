/**
 * @file ipv4_test.cc
 * @brief header 模块单元测试（M0：子网默认值 + IPv4 头编码/校验）。
 *
 * ## 学习目标
 *
 * 1. 理解 **0.0.0.0/0** 默认路由前缀（kIPv4EmptySubnet）；
 * 2. 手工构造最小 IPv4 头并填写 **RFC 1071 校验和**；
 * 3. 用 IsValid / IsChecksumValid 验证编码正确性。
 *
 * ## 运行方式
 *
 * @code
 * cmake --build build
 * ./build/netstack_header_test
 * # 或
 * ctest --test-dir build -R header_ipv4
 * @endcode
 *
 * @see docs/m0.md
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

/**
 * @brief 验证 0.0.0.0/0 子网语义：匹配任意地址，用于默认路由表项。
 *
 * 对标 references/header.IPv4EmptySubnet + tcpip.Subnet Contains。
 */
void TestEmptySubnet() {
  const auto subnet = Subnet::New(IPv4Address{{0, 0, 0, 0}}, 0);
  assert(subnet.has_value());
  assert(subnet->prefix_length == 0);
  assert(subnet->Contains(IPv4Address{{10, 0, 0, 1}}));
  assert(kIPv4EmptySubnet.prefix_length == 0);
}

/**
 * @brief 构造 20 字节最小 IPv4 头（无选项），填写校验和并断言解析一致。
 *
 * 校验和步骤（教学重点）：
 * 1. Encode 后把 checksum 字段置 0；
 * 2. CalculateChecksum() 得到未取反的和；
 * 3. SetChecksumField(~sum) 写入头；
 * 4. IsChecksumValid() 应对整头再算一遍得到 0xFFFF。
 */
void TestEncodeAndValidate() {
  std::vector<uint8_t> buf(20, 0);
  IPv4Header hdr(buf);

  IPv4Fields fields{};
  fields.ihl = 20;
  fields.total_length = 20;
  fields.ttl = 64;
  fields.protocol = 17;  // UDP，仅作字段示例
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
