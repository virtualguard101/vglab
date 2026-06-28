/**
 * @file tun.hh
 * @brief Linux TUN/TAP 设备打开（对标 references/tcpip/link/tun）。
 *
 * M3 实现 **TUN + IFF_NO_PI**（裸 IPv4）；TAP 仅声明占位。
 *
 * @see references/tcpip/link/tun/tun_unsafe.go
 * @see docs/m3.md
 * @see docs/adr/006-m3-tun-linux.md
 */

#pragma once

#include <string_view>

namespace netstack::link {

/**
 * @brief 打开 TUN 设备并设为非阻塞。
 *
 * @param name 接口名（如 "tun0"）；须已由 `ip tuntap` 创建或存在。
 * @return 成功返回 fd；失败返回 -1 并设置 errno。
 *
 * 使用 `IFF_TUN | IFF_NO_PI`，读写为裸 IP 报文（无 4 字节 PI 头）。
 */
int OpenTun(std::string_view name);

/** @brief [—] M3+：IFF_TAP | IFF_NO_PI，需 L2 以太网支持。 */
int OpenTap(std::string_view name);

}  // namespace netstack::link
