/**
 * @file fdbased.hh
 * @brief 基于文件描述符的链路 endpoint（对标 references/tcpip/link/fdbased
 * 子集）。
 *
 * ## 设计选择（ADR 006）
 *
 * - **PollOnce 在 FdEndpoint 上**，不扩展 `Stack::PollNic`，避免改动 Stack 公共
 * API。
 * - demo 主线程：`poll` 等待可读 → `PollOnce` → `Accept`/`Read` 轮询。
 *
 * ## 成员与生命周期
 *
 * - 构造时**接管 fd 所有权**，析构 `close(fd)`。
 * - `Attach` 由 NIC 在 `CreateNIC` 时调用，之后入站方可上交。
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
 * @brief 单 FD 链路 endpoint（TUN 裸 IP 或测试用 socketpair）。
 *
 * 实现 `stack::LinkEndpoint`；入站由调用方驱动 `PollOnce()`，
 * 出站由协议栈同步调用 `WritePacket`。
 */
class FdEndpoint : public stack::LinkEndpoint {
 public:
  /**
   * @param fd 已打开的描述符（TUN 或测试用 pipe/socketpair 一端）。
   * @param mtu 单次 read 缓冲区大小，通常 1500。
   * @param link_addr TUN 无 MAC，常传默认构造的 LinkAddress{}。
   */
  FdEndpoint(int fd, uint32_t mtu, LinkAddress link_addr);
  ~FdEndpoint() override;

  FdEndpoint(const FdEndpoint&) = delete;
  FdEndpoint& operator=(const FdEndpoint&) = delete;

  /**
   * @brief 非阻塞读取当前所有可读入站帧并交付 NIC。
   *
   * 应在 `poll(POLLIN)` 返回后或定时轮询中调用；可多次调用直到无数据。
   */
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
