/**
 * @file tun_tcp_echo.cc
 * @brief M3 验收 demo：TUN + TCP echo（对标 sample/tun_tcp_echo）。
 *
 * 用法：
 *   tun_tcp_echo <tun-device> <local-ipv4> <port>
 *
 * 宿主机另开终端：`nc <local-ipv4> <port>`，输入应回显。
 *
 * 前置（需 root / CAP_NET_ADMIN）：
 *   sudo ip tuntap add user "$USER" mode tun tun0
 *   sudo ip link set tun0 up
 *   sudo ip addr add 10.0.0.2/24 dev tun0
 *   sudo ip route add 10.0.0.1/32 dev tun0
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

  const int fd = netstack::link::OpenTun(tun_name);
  if (fd < 0) {
    std::cerr << "OpenTun(" << tun_name << ") failed: " << ::strerror(errno)
              << '\n';
    return 1;
  }

  netstack::stack::Stack stack;
  stack.AddNetworkProtocol(std::make_unique<netstack::net::ipv4::Protocol>());
  stack.AddTransportProtocol(
      std::make_unique<netstack::transport::tcp::Protocol>());

  auto endpoint = std::make_unique<netstack::link::FdEndpoint>(
      fd, 1500, netstack::LinkAddress{});
  auto* raw = endpoint.get();
  const auto nic_id = stack.CreateNIC(std::move(endpoint));
  if (stack.AddAddress(nic_id, *local_addr).has_value()) {
    std::cerr << "AddAddress failed\n";
    return 1;
  }

  netstack::transport::tcp::Listener listener(&stack);
  if (listener.Listen(nic_id, *local_addr, port).has_value()) {
    std::cerr << "Listen failed\n";
    return 1;
  }

  std::cout << "TCP echo on " << local_addr->ToString() << ':' << port
            << " via " << tun_name << " (Ctrl+C to quit)\n";

  std::vector<netstack::transport::tcp::Connection*> active;

  for (;;) {
    struct pollfd pfd{};
    pfd.fd = fd;
    pfd.events = POLLIN;
    (void)::poll(&pfd, 1, 100);

    raw->PollOnce();

    while (netstack::transport::tcp::Connection* conn = listener.Accept()) {
      active.push_back(conn);
    }

    for (netstack::transport::tcp::Connection* conn : active) {
      const std::vector<uint8_t> data = conn->Read();
      if (!data.empty()) {
        (void)conn->Write(data);
      }
    }
  }
}
