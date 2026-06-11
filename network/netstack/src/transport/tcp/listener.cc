/**
 * @file listener.cc
 * @brief TCP Listener：常驻 LISTEN，SYN 时创建 Connection（M2+ backlog）。
 *
 * @see include/netstack/transport/tcp/listener.hh
 * @see docs/m2+.md
 */

#include "netstack/transport/tcp/listener.hh"

#include "netstack/header/ipv4.hh"
#include "netstack/header/tcp.hh"
#include "netstack/stack/stack.hh"

namespace netstack::transport::tcp {

Listener::Listener(stack::Stack* stack) : stack_(stack) {}

stack::StackResult Listener::Listen(stack::NICID nic_id, IPv4Address local_addr,
                                    uint16_t port, size_t backlog) {
  if (stack_ == nullptr || listening_) {
    return stack::StackError{stack::ErrorCode::kInvalidEndpointState};
  }
  if (!stack_->ReservePort(header::kIPv4ProtocolNumber,
                           header::kTCPProtocolNumber, port)) {
    return stack::StackError{stack::ErrorCode::kInvalidEndpointState};
  }

  stack::TransportEndpointID id{};
  id.local_port = port;
  id.local_address.assign(local_addr.octets.begin(), local_addr.octets.end());

  const auto err = stack_->RegisterTransportEndpoint(
      header::kIPv4ProtocolNumber, header::kTCPProtocolNumber, id, this);
  if (err.has_value()) {
    stack_->ReleasePort(header::kIPv4ProtocolNumber, header::kTCPProtocolNumber,
                        port);
    return err;
  }

  nic_id_ = nic_id;
  local_addr_ = local_addr;
  local_port_ = port;
  backlog_ = backlog;
  listening_ = true;
  return std::nullopt;
}

Connection* Listener::Accept() {
  if (accept_queue_.empty()) {
    return nullptr;
  }
  Connection* conn = accept_queue_.front();
  accept_queue_.pop_front();
  return conn;
}

void Listener::EnqueueEstablished(Connection* conn) {
  accept_queue_.push_back(conn);
}

void Listener::HandlePacket(stack::Route* route, stack::TransportEndpointID id,
                            stack::PacketBuffer pkt) {
  if (!listening_ || stack_ == nullptr || route == nullptr) {
    return;
  }

  auto& data = pkt.Data();
  if (data.size() < header::kTCPMinimumSize) {
    return;
  }

  header::TCPHeader tcp(std::span<uint8_t>(data.data(), data.size()));
  if (!tcp.IsValid(data.size())) {
    return;
  }

  const auto flags = tcp.Flags();
  // RFC 793: LISTEN 只响应无 ACK 的 SYN
  if (!header::HasFlag(flags, header::TCPFlags::kSyn) ||
      header::HasFlag(flags, header::TCPFlags::kAck)) {
    return;
  }

  if (connections_.size() >= backlog_) {
    return;
  }

  auto conn = std::make_unique<Connection>(stack_);
  conn->listener_ = this;
  conn->BeginPassiveOpen(nic_id_, local_addr_, local_port_, route, id,
                         tcp.SequenceNumber());
  connections_.push_back(std::move(conn));
}

}  // namespace netstack::transport::tcp
