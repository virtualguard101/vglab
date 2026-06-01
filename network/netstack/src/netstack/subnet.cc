/**
 * @file subnet.cc
 * @brief Subnet 前缀掩码与归属判断（netstack 核心模块）。
 *
 * ## CIDR 语义
 *
 * - `prefix_length` 表示网络位个数；例如 /8 掩码为 255.0.0.0。
 * - `Subnet::New` 要求 address 的**主机位全 0**（网络地址规范化）。
 * - `Contains` 用 `(addr & mask) == (network & mask)` 判断成员关系。
 *
 * `kIPv4EmptySubnet`（0.0.0.0/0）用于默认路由，见 subnet.hh。
 *
 * @see include/netstack/subnet.hh
 */

#include "netstack/subnet.hh"

#include <limits>
#include <sstream>

namespace netstack {

namespace {

/** @brief 由 CIDR 前缀长度生成 32 位主机序掩码（网络位为 1）。 */
uint32_t PrefixMask(uint8_t prefix_length) {
  if (prefix_length == 0) {
    return 0;
  }
  if (prefix_length >= 32) {
    return 0xffffffffu;
  }
  return std::numeric_limits<uint32_t>::max() << (32 - prefix_length);
}

/** @brief 将 IPv4Address 转为便于位运算的 32 位值（octets[0] 为最高字节）。 */
uint32_t AddressToHostU32(IPv4Address addr) {
  return (static_cast<uint32_t>(addr.octets[0]) << 24) |
         (static_cast<uint32_t>(addr.octets[1]) << 16) |
         (static_cast<uint32_t>(addr.octets[2]) << 8) |
         static_cast<uint32_t>(addr.octets[3]);
}

}  // namespace

std::optional<Subnet> Subnet::New(IPv4Address address, uint8_t prefix_length) {
  if (prefix_length > 32) {
    return std::nullopt;
  }
  const uint32_t mask = PrefixMask(prefix_length);
  const uint32_t addr = AddressToHostU32(address);
  // 主机位必须全 0：addr & ~mask == 0
  if ((addr & ~mask) != 0) {
    return std::nullopt;
  }
  return Subnet{address, prefix_length};
}

bool Subnet::Contains(IPv4Address addr) const {
  const uint32_t mask = PrefixMask(prefix_length);
  const uint32_t net = AddressToHostU32(address);
  const uint32_t a = AddressToHostU32(addr);
  return (a & mask) == (net & mask);
}

std::string Subnet::ToString() const {
  std::ostringstream os;
  os << address.ToString() << '/' << static_cast<unsigned>(prefix_length);
  return os.str();
}

}  // namespace netstack
