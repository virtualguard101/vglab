/**
 * @file listener.cc
 * @brief TCP Listener：常驻 LISTEN，SYN 时创建 Connection（M2+ backlog）。
 *
 * ## 与 Connection 的分工
 *
 * @code
 * 入站 SYN（无 ACK）→ Listener::HandlePacket
 *   → new Connection → BeginPassiveOpen（SYN-RECEIVED）
 * 入站 ACK / 数据     → Connection::HandlePacket（四元组 demux）
 * 握手完成           → accept_queue_ → Accept() 交给应用
 * @endcode
 *
 * ## backlog 语义（教学简化）
 *
 * `connections_.size() >= backlog_` 时**静默丢弃**新 SYN（不发 RST）。
 * 计数含半连接 + 已 ESTABLISHED 未 Accept 的连接。
 *
 * @see include/netstack/transport/tcp/listener.hh
 * @see docs/m2+.md
 * @see docs/tcp-rfc793-states.md
 */

#include "netstack/transport/tcp/listener.hh"

#include "netstack/header/ipv4.hh"
#include "netstack/header/tcp.hh"
#include "netstack/stack/stack.hh"

namespace netstack::transport::tcp {

Listener::Listener(stack::Stack* stack) : stack_(stack) {}

/**
 * @brief passive OPEN：登记监听键 (local_ip, local_port)，进入 LISTEN。
 *
 * 1. PortManager::Reserve 防止端口冲突
 * 2. RegisterTransportEndpoint → demuxer listen_endpoints_
 */
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

/** @brief 取一条已 ESTABLISHED 且尚未被应用取走的连接；FIFO。 */
Connection* Listener::Accept() {
  if (accept_queue_.empty()) {
    return nullptr;
  }
  Connection* conn = accept_queue_.front();
  accept_queue_.pop_front();
  return conn;
}

/** @brief Connection 在 SYN-RECEIVED + ACK 完成后调用。 */
void Listener::EnqueueEstablished(Connection* conn) {
  accept_queue_.push_back(conn);
}

/**
 * @brief 仅处理**纯 SYN**（RFC 793：LISTEN 状态忽略带 ACK 的段）。
 *
 * 通过 backlog 检查后 fork 新 Connection 并 BeginPassiveOpen。
 * 本对象**始终保持 LISTEN**，不转为 ESTABLISHED。
 */
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
