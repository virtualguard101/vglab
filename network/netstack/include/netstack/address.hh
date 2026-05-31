/**
 * @file address.hh
 * @brief IPv4 地址类型：用户可读字符串与报文线格式之间的转换。
 *
 * 本文件属于 **netstack 核心模块**（对标参考实现 `tcpip` 根包中的 `Address`）。
 * 它不解析 IP 头，只表示「四个八位组」这一语义，供
 * header、stack、路由等层复用。
 *
 * @see docs/module-map.md
 * @see references/tcpip/tcpip.go (type Address)
 */

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace netstack {

/**
 * @brief 以网络字节序（大端）存储的 IPv4 地址。
 *
 * 在报文中，源/目的地址各占 4 字节，直接按顺序拷贝到缓冲区即可（见 RFC 791）。
 * 主机上配置地址时常用点分十进制，需经 Parse() 转换后再 WriteWire()。
 */
struct IPv4Address {
  /** @brief 四个八位组，octets[0] 为地址最高字节（如 192.168.1.1 中的 192）。
   */
  std::array<uint8_t, 4> octets{};

  friend bool operator==(const IPv4Address& a, const IPv4Address& b) {
    return a.octets == b.octets;
  }

  /**
   * @brief 从点分十进制字符串解析地址。
   * @param str 例如 `"192.168.1.1"`，不含端口或掩码。
   * @return 解析成功则返回地址；格式非法或八位组 >255 时返回 std::nullopt。
   */
  static std::optional<IPv4Address> Parse(std::string_view str);

  /**
   * @brief 从报文缓冲区读取 4 字节地址（网络序）。
   * @param data 至少指向 4 字节可读内存（通常来自 IPv4 头 offset 12 或 16）。
   */
  static IPv4Address FromWire(const uint8_t* data);

  /**
   * @brief 将地址写入报文缓冲区（网络序）。
   * @param data 至少 4 字节可写内存。
   */
  void WriteWire(uint8_t* data) const;

  /** @brief 转为点分十进制，便于日志与测试断言。 */
  std::string ToString() const;
};

/** @brief IPv4 有限广播地址
 * 255.255.255.255（references/header.IPv4Broadcast）。 */
inline constexpr IPv4Address kIPv4Broadcast{{0xff, 0xff, 0xff, 0xff}};

/**
 * @brief 「任意」/meta 地址 0.0.0.0（references/header.IPv4Any）。
 *
 * 常用于「绑定任意本地地址」或构造 0.0.0.0/0 默认路由前缀（见 subnet.hh）。
 */
inline constexpr IPv4Address kIPv4Any{{0, 0, 0, 0}};

}  // namespace netstack
