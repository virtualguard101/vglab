/**
 * @file tun_tcp_echo.cc
 * @brief M3 验收 demo：TUN + TCP echo（对标 sample/tun_tcp_echo）。
 *
 * ## 与参考 demo 的差异
 *
 * | 参考 (Go) | 本 demo |
 * |-----------|---------|
 * | NewEndpoint + waiter 阻塞 | Listener + Connection + poll 轮询 |
 * | SetRouteTable 默认路由 | 单 NIC，出站写同一 TUN |
 * | ARP/IPv6 | 仅 IPv4 + TCP |
 *
 * ## 用法
 *
 * @code
 * tun_tcp_echo <tun-device> <local-ipv4> <port>
 * # 另一终端：nc <local-ipv4> <port>
 * @endcode
 *
 * ## 宿主机配置（重要）
 *
 * **勿**在宿主机 `ip addr` 与栈 `AddAddress` 使用同一 IP，否则本机 nc 走
 * local 表不进 TUN。推荐：
 *
 * @code
 * sudo ip tuntap add user "$USER" mode tun tun0
 * sudo ip link set tun0 up
 * sudo ip addr add 10.0.0.2/24 dev tun0
 * sudo ip route add 10.0.0.1/32 dev tun0
 * @endcode
 *
 * @see docs/m3.md
 * @see references/tcpip/sample/tun_tcp_echo/main.go
 */

#include <poll.h>

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "netstack/address.hh"
#include "netstack/link/fdbased.hh"
#include "netstack/link/tun.hh"
#include "netstack/net/ipv4/protocol.hh"
#include "netstack/stack/stack.hh"
#include "netstack/transport/tcp/connection.hh"
#include "netstack/transport/tcp/listener.hh"
#include "netstack/transport/tcp/protocol.hh"

namespace {

void Usage(const char* prog) {
  std::cerr << "Usage: " << prog << " <tun-device> <local-ipv4> <local-port>\n"
            << "Example: " << prog << " tun0 10.0.0.1 8080\n";
}

uint16_t ParsePort(const char* s) {
  char* end = nullptr;
  errno = 0;
  const unsigned long v = std::strtoul(s, &end, 10);
  if (errno != 0 || end == s || *end != '\0' || v > 65535) {
    return 0;
  }
  return static_cast<uint16_t>(v);
}

}  // namespace

int main(int argc, char* argv[]) {
  if (argc != 4) {
    Usage(argv[0]);
    return 1;
  }

  const std::string tun_name = argv[1];
  const auto local_addr = netstack::IPv4Address::Parse(argv[2]);
  if (!local_addr.has_value()) {
    std::cerr << "Bad IP address: " << argv[2] << '\n';
    return 1;
  }
  const uint16_t port = ParsePort(argv[3]);
  if (port == 0) {
    std::cerr << "Bad port: " << argv[3] << '\n';
    return 1;
  }

  // --- 1. 打开 TUN（裸 IP、非阻塞）---
  const int fd = netstack::link::OpenTun(tun_name);
  if (fd < 0) {
    std::cerr << "OpenTun(" << tun_name << ") failed: " << ::strerror(errno)
              << '\n';
    return 1;
  }

  // --- 2. 组装协议栈：IPv4 + TCP ---
  netstack::stack::Stack stack;
  stack.AddNetworkProtocol(std::make_unique<netstack::net::ipv4::Protocol>());
  stack.AddTransportProtocol(
      std::make_unique<netstack::transport::tcp::Protocol>());

  // --- 3. NIC = FdEndpoint；fd 所有权交给 FdEndpoint ---
  auto endpoint = std::make_unique<netstack::link::FdEndpoint>(
      fd, 1500, netstack::LinkAddress{});
  auto* raw = endpoint.get();
  const auto nic_id = stack.CreateNIC(std::move(endpoint));
  if (stack.AddAddress(nic_id, *local_addr).has_value()) {
    std::cerr << "AddAddress failed\n";
    return 1;
  }

  // --- 4. 被动打开：监听 local_addr:port ---
  netstack::transport::tcp::Listener listener(&stack);
  if (listener.Listen(nic_id, *local_addr, port).has_value()) {
    std::cerr << "Listen failed\n";
    return 1;
  }

  std::cout << "TCP echo on " << local_addr->ToString() << ':' << port
            << " via " << tun_name << " (Ctrl+C to quit)\n";

  // Connection 由 Listener 持有 unique_ptr；此处只存裸指针做 echo
  std::vector<netstack::transport::tcp::Connection*> active;

  // --- 5. 事件循环（无 waiter：poll + 轮询）---
  for (;;) {
    struct pollfd pfd{};
    pfd.fd = fd;
    pfd.events = POLLIN;
    // 100ms 超时：无包时仍轮询 Accept/Read，避免纯阻塞
    (void)::poll(&pfd, 1, 100);

    // 入站：read TUN → DeliverNetworkPacket → … → Listener/Connection
    raw->PollOnce();

    // 新完成的连接移入 active 列表
    while (netstack::transport::tcp::Connection* conn = listener.Accept()) {
      active.push_back(conn);
    }

    // 对已连接套接字做 echo：Read 缓冲 → Write 回射
    for (netstack::transport::tcp::Connection* conn : active) {
      const std::vector<uint8_t> data = conn->Read();
      if (!data.empty()) {
        (void)conn->Write(data);
      }
    }
  }
}
