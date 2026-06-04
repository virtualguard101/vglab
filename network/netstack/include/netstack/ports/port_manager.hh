/**
 * @file port_manager.hh
 * @brief 简化版端口占用表（M1）。
 *
 * @see references/tcpip/ports/ports.go
 */

#pragma once

#include <cstdint>
#include <unordered_set>

#include "netstack/stack/registration.hh"

namespace netstack::ports {

class PortManager {
 public:
  bool Reserve(stack::NetworkProtocolNumber net,
               stack::TransportProtocolNumber trans, uint16_t port);
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
