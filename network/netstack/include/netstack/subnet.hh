/**
 * @file subnet.hh
 * @brief IPv4 子网（网络前缀）：路由表与地址归属判断。
 *
 * 本文件属于 **netstack 核心模块**（对标 `tcpip.Subnet` / `NewSubnet`）。
 * 子网描述「哪些地址属于同一网段」，与 **header** 层（单包编解码）分离：
 * - header：单帧 IPv4 头的字段
 * - subnet：跨包的路由/匹配语义
 *
 * @see docs/module-map.md
 * @see references/tcpip/tcpip.go (Subnet)
 */

#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "netstack/address.hh"

namespace netstack {

/**
 * @brief 由网络地址 + 前缀长度定义的 IPv4 子网。
 *
 * 例如 address=10.0.0.0、prefix_length=8 表示 10.0.0.0/8。
 * 参考实现用 Address + AddressMask；此处用前缀长度便于教学与配置。
 */
struct Subnet {
  /** @brief 子网 ID（网络号），主机位必须为 0（由 New 校验）。 */
  IPv4Address address{};

  /**
   * @brief 前缀长度（CIDR），0～32。
   * - 0 表示掩码 0.0.0.0，即匹配所有地址（用于默认路由 0.0.0.0/0）。
   * - 32 表示主机路由 /32。
   */
  uint8_t prefix_length{};

  friend bool operator==(const Subnet& a, const Subnet& b) {
    return a.address == b.address && a.prefix_length == b.prefix_length;
  }

  /**
   * @brief 构造子网并校验 address 是否已按前缀对齐（主机位为 0）。
   * @param address 网络地址。
   * @param prefix_length 前缀位数；大于 32 时失败。
   * @return 合法则返回 Subnet；若 address 含主机位则返回 std::nullopt。
   *
   * @note 对标 references/tcpip.NewSubnet：要求 a[i] & ~mask[i] == 0。
   */
  static std::optional<Subnet> New(IPv4Address address, uint8_t prefix_length);

  /**
   * @brief 判断地址是否落在本子网内。
   * @param addr 待检测的 IPv4 地址。
   * @return 若 (addr & mask) == (network & mask) 则为 true。
   */
  bool Contains(IPv4Address addr) const;

  /** @brief 形如 `"10.0.0.0/8"` 的字符串。 */
  std::string ToString() const;
};

/**
 * @brief 空前缀子网 0.0.0.0/0，用作路由表「默认路由」的目的地。
 *
 * 语义对标 references/header.IPv4EmptySubnet（Go 中由 IPv4Any + 掩码 0.0.0.0
 * 构造）。 测试里常见写法：SetRouteTable({{Destination: kIPv4EmptySubnet, NIC:
 * 1}})。
 */
inline constexpr Subnet kIPv4EmptySubnet{kIPv4Any, 0};

}  // namespace netstack
