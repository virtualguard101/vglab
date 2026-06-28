/**
 * @file tun.hh
 * @brief Linux TUN/TAP 设备打开（对标 references/tcpip/link/tun）。
 *
 * ## 职责边界
 *
 * 本模块**只负责打开字符设备 fd**，不实现协议栈逻辑。
 * 读/写与 `DeliverNetworkPacket` 由 `link::FdEndpoint` 完成。
 *
 * ## 典型调用链（M3 demo）
 *
 * @code
 * int fd = OpenTun("tun0");
 * auto ep = std::make_unique<FdEndpoint>(fd, 1500, LinkAddress{});
 * stack.CreateNIC(std::move(ep));
 * @endcode
 *
 * 设备须先用 `ip tuntap add` 创建；见 docs/m3.md 宿主机配置。
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
 * @return 成功返回 fd（调用方交给 FdEndpoint 管理）；失败 -1 并设 errno。
 *
 * 标志：`IFF_TUN | IFF_NO_PI` — 读写为裸 IPv4/IPv6 报文，无 4 字节 PI 头。
 */
int OpenTun(std::string_view name);

/**
 * @brief [—] M3+：以太网 TAP 模式。
 *
 * 需 L2 demux、`MaxHeaderLength()==14`、ARP 等；见 docs/m3.md「TAP 路径」。
 */
int OpenTap(std::string_view name);

}  // namespace netstack::link
