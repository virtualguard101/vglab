/**
 * @file fdbased.hh
 * @brief 基于文件描述符的链路 endpoint（对标 references/tcpip/link/fdbased
 * 子集）。
 *
 * - **WritePacket**：`write(2)` 写出裸 IPv4（TUN 模式，MaxHeaderLength==0）。
 * - **PollOnce**：非阻塞 `read` 循环，交付 `DeliverNetworkPacket`。
 *
 * demo 主循环显式调用 PollOnce，不扩展 Stack API。
 *
 * @see references/tcpip/link/fdbased/endpoint.go
 * @see docs/m3.md
 */

#pragma once

#include <cstdint>
#include <vector>

#include "netstack/address.hh"
#include "netstack/stack/registration.hh"

namespace netstack::link {

/**
 * @brief 单 FD 链路 endpoint（TUN 裸 IP）。
 *
 * 构造后取得 fd 所有权；析构时 close(fd)。
 */
class FdEndpoint : public stack::LinkEndpoint {
 public:
  FdEndpoint(int fd, uint32_t mtu, LinkAddress link_addr);
  ~FdEndpoint() override;

  FdEndpoint(const FdEndpoint&) = delete;
  FdEndpoint& operator=(const FdEndpoint&) = delete;

  /** @brief 非阻塞读取所有可读入站包并上交 NIC。 */
  void PollOnce();

  uint32_t MTU() const override;
  stack::LinkEndpointCapabilities Capabilities() const override;
  uint16_t MaxHeaderLength() const override;
  LinkAddress LinkAddr() const override;

  stack::StackResult WritePacket(stack::Route* route, const stack::GSO* gso,
                                 stack::NetworkProtocolNumber protocol,
                                 stack::PacketBuffer pkt) override;

  void Attach(stack::NetworkDispatcher* dispatcher) override;
  bool IsAttached() const override;

 private:
  int fd_{-1};
  uint32_t mtu_{1500};
  LinkAddress link_addr_{};
  stack::NetworkDispatcher* dispatcher_{nullptr};
  std::vector<uint8_t> read_buf_{};
};

}  // namespace netstack::link
