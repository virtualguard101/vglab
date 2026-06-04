/**
 * @file endpoint.cc
 * @brief UDP echo endpoint（M1）。
 */

#include "netstack/transport/udp/endpoint.hh"

#include <vector>

#include "netstack/header/ipv4.hh"
#include "netstack/header/udp.hh"
#include "netstack/net/ipv4/send.hh"
#include "netstack/stack/stack.hh"

namespace netstack::transport::udp {

Endpoint::Endpoint(stack::Stack* stack) : stack_(stack) {}

stack::StackResult Endpoint::Bind(stack::NICID nic_id, IPv4Address local_addr,
                                  uint16_t port) {
  if (stack_ == nullptr) {
    return stack::StackError{stack::ErrorCode::kInvalidEndpointState};
  }
  if (!stack_->ReservePort(header::kIPv4ProtocolNumber,
                           header::kUDPProtocolNumber, port)) {
    return stack::StackError{stack::ErrorCode::kInvalidEndpointState};
  }

  stack::TransportEndpointID id{};
  id.local_port = port;
  id.local_address.assign(local_addr.octets.begin(), local_addr.octets.end());

  const auto err = stack_->RegisterTransportEndpoint(
      header::kIPv4ProtocolNumber, header::kUDPProtocolNumber, id, this);
  if (err.has_value()) {
    stack_->ReleasePort(header::kIPv4ProtocolNumber, header::kUDPProtocolNumber,
                        port);
    return err;
  }

  nic_id_ = nic_id;
  local_addr_ = local_addr;
  port_ = port;
  bound_ = true;
  return std::nullopt;
}

void Endpoint::HandlePacket(stack::Route* route, stack::TransportEndpointID id,
                          stack::PacketBuffer pkt) {
  (void)id;
  if (!bound_ || stack_ == nullptr || route == nullptr) {
    return;
  }

  auto& data = pkt.Data();
  if (data.size() < header::kUDPMinimumSize) {
    return;
  }

  header::UDPHeader udp(std::span<uint8_t>(data.data(), data.size()));
  if (!udp.IsValid(data.size())) {
    return;
  }

  const uint16_t ulen = udp.Length();
  if (ulen < header::kUDPMinimumSize || ulen > data.size()) {
    return;
  }

  const std::vector<uint8_t> payload(data.begin() + header::kUDPMinimumSize,
                                     data.begin() + ulen);

  std::vector<uint8_t> l4(header::kUDPMinimumSize + payload.size(), 0);
  header::UDPHeader out_hdr(std::span<uint8_t>(l4.data(), l4.size()));
  header::UDPFields fields{};
  fields.src_port = port_;
  fields.dst_port = id.remote_port;
  fields.length = static_cast<uint16_t>(l4.size());
  fields.checksum = 0;
  out_hdr.Encode(fields);
  std::copy(payload.begin(), payload.end(),
            l4.begin() + header::kUDPMinimumSize);

  stack::Route reply{};
  reply.net_proto = header::kIPv4ProtocolNumber;
  reply.local_address = route->local_address;
  reply.remote_address = route->remote_address;

  (void)net::ipv4::SendPacket(*stack_, nic_id_, reply,
                              header::kUDPProtocolNumber, std::move(l4));
}

}  // namespace netstack::transport::udp
