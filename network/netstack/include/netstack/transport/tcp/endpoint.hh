/**
 * @file endpoint.hh
 * @brief TCP endpoint：Listen、握手、按序收发包、FIN 关闭（M2）。
 *
 * @see docs/m2.md
 */

#pragma once

#include <cstdint>
#include <vector>

#include "netstack/address.hh"
#include "netstack/seqnum.hh"
#include "netstack/stack/registration.hh"
#include "netstack/stack/transport.hh"
#include "netstack/transport/tcp/state.hh"

namespace netstack::stack {
class Stack;
}

namespace netstack::transport::tcp {

class Endpoint : public stack::TransportEndpoint {
 public:
  explicit Endpoint(stack::Stack* stack);

  stack::StackResult Listen(stack::NICID nic_id, IPv4Address local_addr,
                            uint16_t port);

  TcpState State() const { return state_; }
  seqnum::Value Iss() const { return iss_; }

  std::vector<uint8_t> Read();
  stack::StackResult Write(std::span<const uint8_t> data);
  void Close();

  void HandlePacket(stack::Route* route, stack::TransportEndpointID id,
                    stack::PacketBuffer pkt) override;

 private:
  void SendSegment(stack::Route* route, uint8_t flags, uint32_t seq,
                   uint32_t ack, std::span<const uint8_t> payload);

  stack::Stack* stack_{nullptr};
  stack::NICID nic_id_{};
  IPv4Address local_addr_{};
  uint16_t local_port_{};
  uint16_t remote_port_{};
  stack::Address remote_addr_{};

  TcpState state_{TcpState::kClosed};
  seqnum::Value iss_{100000};
  seqnum::Value rcv_nxt_{};
  seqnum::Value snd_nxt_{};
  std::vector<uint8_t> rcv_buf_{};
};

}  // namespace netstack::transport::tcp
