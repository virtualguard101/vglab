/**
 * @file endpoint.hh
 * @brief UDP endpoint：Bind + echo 回射（M1）。
 */

#pragma once

#include <cstdint>

#include "netstack/address.hh"
#include "netstack/stack/registration.hh"
#include "netstack/stack/transport.hh"

namespace netstack::stack {
class Stack;
}

namespace netstack::transport::udp {

class Endpoint : public stack::TransportEndpoint {
 public:
  explicit Endpoint(stack::Stack* stack);

  stack::StackResult Bind(stack::NICID nic_id, IPv4Address local_addr,
                          uint16_t port);

  void HandlePacket(stack::Route* route, stack::TransportEndpointID id,
                    stack::PacketBuffer pkt) override;

 private:
  stack::Stack* stack_{nullptr};
  stack::NICID nic_id_{};
  IPv4Address local_addr_{};
  uint16_t port_{};
  bool bound_{false};
};

}  // namespace netstack::transport::udp
