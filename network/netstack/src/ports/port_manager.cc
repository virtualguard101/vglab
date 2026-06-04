/**
 * @file port_manager.cc
 * @brief 端口 Reserve/Release 实现。
 */

#include "netstack/ports/port_manager.hh"

namespace netstack::ports {

size_t PortManager::PortKeyHash::operator()(const PortKey& k) const {
  return std::hash<uint32_t>{}((static_cast<uint32_t>(k.net) << 16) |
                               (static_cast<uint32_t>(k.trans) << 8) | k.port);
}

bool PortManager::Reserve(stack::NetworkProtocolNumber net,
                          stack::TransportProtocolNumber trans, uint16_t port) {
  PortKey key{net, trans, port};
  if (reserved_.count(key) != 0) {
    return false;
  }
  reserved_.insert(key);
  return true;
}

void PortManager::Release(stack::NetworkProtocolNumber net,
                          stack::TransportProtocolNumber trans, uint16_t port) {
  reserved_.erase(PortKey{net, trans, port});
}

}  // namespace netstack::ports
