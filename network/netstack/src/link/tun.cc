/**
 * @file tun.cc
 * @brief Linux TUN/TAP 打开实现（对标
 * references/tcpip/link/tun/tun_unsafe.go）。
 *
 * ## 教学要点
 *
 * TUN 是用户态与内核之间的「虚拟网卡」管道：
 *
 * @code
 *  宿主 nc ──► 内核路由 ──► write to tun fd ──► read(2) ──► userspace 栈
 *  userspace 栈 ──► write(2) ──► tun fd ──► 内核 ──► 对端 socket
 * @endcode
 *
 * 打开步骤（与 Go 参考一致）：
 * 1. `open("/dev/net/tun", O_RDWR)`
 * 2. `ioctl(TUNSETIFF)` 绑定接口名与模式（TUN/TAP、PI 头等）
 * 3. `fcntl(O_NONBLOCK)` — demo 用 poll + PollOnce，避免 read 卡死
 *
 * ## IFF_NO_PI 为何必须
 *
 * 默认 TUN 可在每帧前加 4 字节 Packet Information（flags + proto）。
 * 教学栈与 channel 约定 **裸 IPv4**（`MaxHeaderLength()==0`），
 * 必须设 `IFF_NO_PI`，否则 IPv4 头从偏移 4 开始，解析全错。
 *
 * @see include/netstack/link/tun.hh
 * @see docs/m3.md
 * @see docs/adr/006-m3-tun-linux.md
 */

#include "netstack/link/tun.hh"

#include <cerrno>
#include <cstdint>
#include <cstring>

#ifndef __linux__
#error "link/tun requires Linux"
#endif

#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace netstack::link {

namespace {

/** @brief 在已有 fd 上设置 O_NONBLOCK；失败时保留 errno。 */
int SetNonblock(int fd) {
  const int fl = ::fcntl(fd, F_GETFL, 0);
  if (fl < 0) {
    return -1;
  }
  if (::fcntl(fd, F_SETFL, fl | O_NONBLOCK) < 0) {
    return -1;
  }
  return 0;
}

/**
 * @brief 通用打开路径：TUN 或 TAP，由 flags 区分。
 *
 * @param name 接口名，长度须 < IFNAMSIZ（16）。
 * @param flags `IFF_TUN|IFF_NO_PI` 或 `IFF_TAP|IFF_NO_PI` 等。
 */
int OpenWithFlags(std::string_view name, std::uint16_t flags) {
  if (name.size() >= IFNAMSIZ) {
    errno = EINVAL;
    return -1;
  }

  const int fd = ::open("/dev/net/tun", O_RDWR);
  if (fd < 0) {
    return -1;
  }

  struct ifreq ifr{};
  std::memcpy(ifr.ifr_name, name.data(), name.size());
  ifr.ifr_flags = static_cast<short>(flags);

  if (::ioctl(fd, TUNSETIFF, &ifr) < 0) {
    const int err = errno;
    ::close(fd);
    errno = err;
    return -1;
  }

  if (SetNonblock(fd) < 0) {
    const int err = errno;
    ::close(fd);
    errno = err;
    return -1;
  }

  return fd;
}

}  // namespace

/** @brief 裸 IP 模式 TUN；M3 验收使用此函数。 */
int OpenTun(std::string_view name) {
  return OpenWithFlags(name, IFF_TUN | IFF_NO_PI);
}

/** @brief 以太网帧模式 TAP；M3+ 实现 L2 后启用。 */
int OpenTap(std::string_view name) {
  return OpenWithFlags(name, IFF_TAP | IFF_NO_PI);
}

}  // namespace netstack::link
