/**
 * @file port_manager.cc
 * @brief 端口 Reserve/Release 实现（对标 ports.PortManager 子集）。
 *
 * ## 用途
 *
 * - TCP `Listen` / `Connect`、UDP `Bind` 前 Reserve，防止同一 (L3, L4, port)
 * 重复绑定。
 * - 关闭或 Listen 失败时 Release。
 *
 * 教学版**不**实现临时端口自动分配（ephemeral port）；测试显式指定端口。
 *
 * @see include/netstack/ports/port_manager.hh
 */

#include "netstack/ports/port_manager.hh"

namespace netstack::ports {

/** @brief 哈希键：(network_proto, transport_proto, port)。 */
size_t PortManager::PortKeyHash::operator()(const PortKey& k) const {
  return std::hash<uint32_t>{}((static_cast<uint32_t>(k.net) << 16) |
                               (static_cast<uint32_t>(k.trans) << 8) | k.port);
}

/**
 * @brief 占用端口；已占用返回 false。
 *
 * @param net 如 kIPv4ProtocolNumber (0x0800)
 * @param trans 如 kTCPProtocolNumber (6)
 */
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
