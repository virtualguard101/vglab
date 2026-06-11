/**
 * @file port_manager.hh
 * @brief 简化版端口占用表（M1）。
 *
 * 参考实现 `tcpip/ports` 还负责临时端口分配、reuse、按 NIC 绑定等；
 * M1 仅保证 **同一 (网络协议, 传输协议, 端口号) 不可重复 Reserve**，
 * 供 `udp::Endpoint::Bind` 使用。
 *
 * @see references/tcpip/ports/ports.go
 * @see docs/m1.md
 */

#pragma once

#include <cstdint>
#include <unordered_set>

#include "netstack/stack/registration.hh"

namespace netstack::ports {

/**
 * @brief 端口占用登记簿（非完整 PortManager）。
 */
class PortManager {
 public:
  /**
   * @brief 尝试占用端口；已占用则返回 false。
   * @param net 网络层协议值，如 IPv4 的 0x0800（此处 Bind 路径用
   * kIPv4ProtocolNumber）。
   * @param trans 传输协议号，如 UDP 的 17。
   * @param port 端口号。
   * @return 是否成功占用。
   */
  bool Reserve(stack::NetworkProtocolNumber net,
               stack::TransportProtocolNumber trans, uint16_t port);

  /** @brief 释放端口（Bind 失败回滚或 endpoint 关闭时用）。 */
  void Release(stack::NetworkProtocolNumber net,
               stack::TransportProtocolNumber trans, uint16_t port);

 private:
  struct PortKey {
    stack::NetworkProtocolNumber net{};
    stack::TransportProtocolNumber trans{};
    uint16_t port{};

    bool operator==(const PortKey& o) const {
      return net == o.net && trans == o.trans && port == o.port;
    }
  };

  struct PortKeyHash {
    size_t operator()(const PortKey& k) const;
  };

  std::unordered_set<PortKey, PortKeyHash> reserved_{};
};

}  // namespace netstack::ports
