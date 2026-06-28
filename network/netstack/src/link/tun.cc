/**
 * @file tun.cc
 * @brief Linux TUN/TAP 打开实现（对标 tun_unsafe.go）。
 *
 * ## 要点
 *
 * - `/dev/net/tun` + `ioctl(TUNSETIFF)` 绑定接口名。
 * - **IFF_NO_PI**：禁止 packet information 头，与 channel 裸 IPv4 一致。
 * - `fcntl(O_NONBLOCK)`：demo 用 poll + PollOnce，避免阻塞。
 *
 * @see include/netstack/link/tun.hh
 * @see docs/m3.md
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

int OpenTun(std::string_view name) {
  return OpenWithFlags(name, IFF_TUN | IFF_NO_PI);
}

int OpenTap(std::string_view name) {
  return OpenWithFlags(name, IFF_TAP | IFF_NO_PI);
}

}  // namespace netstack::link
