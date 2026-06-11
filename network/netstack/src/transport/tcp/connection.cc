/**
 * @file connection.cc
 * @brief 单条 TCP 连接状态机（M2+：SYN-SENT、四元组 demux、被动/主动打开）。
 *
 * @see include/netstack/transport/tcp/connection.hh
 * @see docs/tcp-rfc793-states.md
 */

#include "netstack/transport/tcp/connection.hh"

#include <algorithm>
#include <atomic>
#include <cstring>

#include "netstack/header/checksum.hh"
#include "netstack/header/ipv4.hh"
#include "netstack/header/tcp.hh"
#include "netstack/net/ipv4/send.hh"
#include "netstack/stack/stack.hh"
#include "netstack/transport/tcp/listener.hh"

namespace netstack::transport::tcp {

namespace {

std::atomic<uint32_t> g_next_iss{100000};

IPv4Address FromStackAddress(const stack::Address& addr) {
  IPv4Address out{};
  if (addr.size() >= 4) {
    for (size_t i = 0; i < 4; ++i) {
      out.octets[i] = addr[i];
    }
  }
  return out;
}

uint16_t ComputeTcpChecksum(const IPv4Address& src, const IPv4Address& dst,
                            std::span<const uint8_t> segment) {
  uint8_t pseudo[12]{};
  std::memcpy(pseudo, src.octets.data(), 4);
  std::memcpy(pseudo + 4, dst.octets.data(), 4);
  pseudo[9] = header::kTCPProtocolNumber;
  const uint16_t tcp_len = static_cast<uint16_t>(segment.size());
  pseudo[10] = static_cast<uint8_t>(tcp_len >> 8);
  pseudo[11] = static_cast<uint8_t>(tcp_len);

  uint16_t sum = header::Checksum(pseudo);
  sum = header::ChecksumCombine(sum, header::Checksum(segment));
  return static_cast<uint16_t>(~sum);
}

}  // namespace

stack::TransportEndpointID Connection::EndpointId() const {
  stack::TransportEndpointID id{};
  id.local_port = local_port_;
  id.local_address.assign(local_addr_.octets.begin(), local_addr_.octets.end());
  id.remote_port = remote_port_;
  id.remote_address = remote_addr_;
  return id;
}

Connection::Connection(stack::Stack* stack) : stack_(stack) {
  iss_ = g_next_iss.fetch_add(1000);
}

stack::StackResult Connection::Connect(stack::NICID nic_id,
                                       IPv4Address local_addr,
                                       uint16_t local_port,
                                       IPv4Address remote_addr,
                                       uint16_t remote_port) {
  if (stack_ == nullptr || state_ != TcpState::kClosed) {
    return stack::StackError{stack::ErrorCode::kInvalidEndpointState};
  }
  if (!stack_->ReservePort(header::kIPv4ProtocolNumber,
                           header::kTCPProtocolNumber, local_port)) {
    return stack::StackError{stack::ErrorCode::kInvalidEndpointState};
  }

  nic_id_ = nic_id;
  local_addr_ = local_addr;
  local_port_ = local_port;
  remote_port_ = remote_port;
  remote_addr_.assign(remote_addr.octets.begin(), remote_addr.octets.end());
  owns_port_ = true;

  stack::Route route{};
  route.net_proto = header::kIPv4ProtocolNumber;
  route.local_address.assign(local_addr_.octets.begin(),
                             local_addr_.octets.end());
  route.remote_address = remote_addr_;

  snd_nxt_ = seqnum::Add(iss_, 1);
  SendSegment(&route, static_cast<uint8_t>(header::TCPFlags::kSyn), iss_, 0,
              {});

  const auto reg = RegisterConnected();
  if (reg.has_value()) {
    stack_->ReleasePort(header::kIPv4ProtocolNumber, header::kTCPProtocolNumber,
                        local_port);
    return reg;
  }

  // RFC 793: active OPEN — CLOSED → SYN-SENT
  state_ = TcpState::kSynSent;
  return std::nullopt;
}

void Connection::BeginPassiveOpen(stack::NICID nic_id, IPv4Address local_addr,
                                  uint16_t local_port, stack::Route* route,
                                  stack::TransportEndpointID id,
                                  uint32_t client_seq) {
  nic_id_ = nic_id;
  local_addr_ = local_addr;
  local_port_ = local_port;
  owns_port_ = false;
  remote_port_ = id.remote_port;
  remote_addr_ = id.remote_address;
  rcv_nxt_ = seqnum::Add(client_seq, 1);
  snd_nxt_ = seqnum::Add(iss_, 1);

  SendSegment(
      route,
      static_cast<uint8_t>(header::TCPFlags::kSyn | header::TCPFlags::kAck),
      iss_, rcv_nxt_, {});
  (void)RegisterConnected();
  state_ = TcpState::kSynReceived;
}

std::vector<uint8_t> Connection::Read() {
  std::vector<uint8_t> out;
  out.swap(rcv_buf_);
  return out;
}

stack::StackResult Connection::Write(std::span<const uint8_t> data) {
  if (stack_ == nullptr || state_ != TcpState::kEstablished) {
    return stack::StackError{stack::ErrorCode::kInvalidEndpointState};
  }

  stack::Route route{};
  route.net_proto = header::kIPv4ProtocolNumber;
  route.local_address.assign(local_addr_.octets.begin(),
                             local_addr_.octets.end());
  route.remote_address = remote_addr_;

  const auto flags =
      static_cast<uint8_t>(header::TCPFlags::kAck | header::TCPFlags::kPsh);
  SendSegment(&route, flags, snd_nxt_, rcv_nxt_, data);
  snd_nxt_ = seqnum::Add(snd_nxt_, static_cast<seqnum::Size>(data.size()));
  return std::nullopt;
}

void Connection::Close() {
  if (stack_ == nullptr) {
    return;
  }
  if (state_ == TcpState::kEstablished || state_ == TcpState::kCloseWait) {
    stack::Route route{};
    route.net_proto = header::kIPv4ProtocolNumber;
    route.local_address.assign(local_addr_.octets.begin(),
                               local_addr_.octets.end());
    route.remote_address = remote_addr_;
    SendSegment(
        &route,
        static_cast<uint8_t>(header::TCPFlags::kFin | header::TCPFlags::kAck),
        snd_nxt_, rcv_nxt_, {});
    snd_nxt_ = seqnum::Add(snd_nxt_, 1);
    state_ = TcpState::kLastAck;
  }
}

void Connection::SendSegment(stack::Route* route, uint8_t flags, uint32_t seq,
                             uint32_t ack, std::span<const uint8_t> payload) {
  if (stack_ == nullptr || route == nullptr) {
    return;
  }

  std::vector<uint8_t> l4(header::kTCPMinimumSize + payload.size(), 0);
  header::TCPHeader tcp(std::span<uint8_t>(l4.data(), l4.size()));
  header::TCPFields fields{};
  fields.src_port = local_port_;
  fields.dst_port = remote_port_;
  fields.seq_num = seq;
  fields.ack_num = ack;
  fields.data_offset = header::kTCPMinimumSize;
  fields.flags = flags;
  fields.window_size = 65535;
  fields.checksum = 0;
  tcp.Encode(fields);
  std::copy(payload.begin(), payload.end(),
            l4.begin() + header::kTCPMinimumSize);

  const auto src = FromStackAddress(route->local_address);
  const auto dst = FromStackAddress(route->remote_address);
  fields.checksum = ComputeTcpChecksum(src, dst, l4);
  tcp.Encode(fields);

  (void)net::ipv4::SendPacket(*stack_, nic_id_, *route,
                              header::kTCPProtocolNumber, std::move(l4));
}

stack::StackResult Connection::RegisterConnected() {
  if (stack_ == nullptr || demux_registered_) {
    return std::nullopt;
  }
  const auto err = stack_->RegisterConnectedEndpoint(
      header::kIPv4ProtocolNumber, header::kTCPProtocolNumber, EndpointId(),
      this);
  if (!err.has_value()) {
    demux_registered_ = true;
  }
  return err;
}

void Connection::UnregisterConnected() {
  if (stack_ == nullptr || !demux_registered_) {
    return;
  }
  stack_->UnregisterConnectedEndpoint(header::kIPv4ProtocolNumber,
                                      header::kTCPProtocolNumber, EndpointId());
  demux_registered_ = false;
}

void Connection::NotifyEstablished() {
  if (listener_ != nullptr) {
    listener_->EnqueueEstablished(this);
  }
}

void Connection::HandlePacket(stack::Route* route,
                              stack::TransportEndpointID id,
                              stack::PacketBuffer pkt) {
  (void)id;
  if (stack_ == nullptr || route == nullptr) {
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
  const auto seq = tcp.SequenceNumber();
  const auto ack = tcp.AcknowledgmentNumber();
  const auto payload = tcp.Payload();

  switch (state_) {
    case TcpState::kSynSent: {
      // RFC 793: SYN-SENT + RCV SYN|ACK → ESTABLISHED（发 ACK）
      if (!header::HasFlag(flags, header::TCPFlags::kSyn) ||
          !header::HasFlag(flags, header::TCPFlags::kAck)) {
        return;
      }
      if (ack != snd_nxt_) {
        return;
      }
      rcv_nxt_ = seqnum::Add(seq, 1);
      SendSegment(route, static_cast<uint8_t>(header::TCPFlags::kAck), snd_nxt_,
                  rcv_nxt_, {});
      state_ = TcpState::kEstablished;
      break;
    }
    case TcpState::kSynReceived: {
      // RFC 793: SYN-RECEIVED + RCV ACK → ESTABLISHED
      if (!header::HasFlag(flags, header::TCPFlags::kAck) || ack != snd_nxt_) {
        return;
      }
      state_ = TcpState::kEstablished;
      NotifyEstablished();
      break;
    }
    case TcpState::kEstablished: {
      if (header::HasFlag(flags, header::TCPFlags::kFin)) {
        if (seq != rcv_nxt_) {
          return;
        }
        rcv_nxt_ = seqnum::Add(rcv_nxt_, 1);
        SendSegment(route, static_cast<uint8_t>(header::TCPFlags::kAck),
                    snd_nxt_, rcv_nxt_, {});
        state_ = TcpState::kCloseWait;
        break;
      }
      if (!payload.empty()) {
        if (seq != rcv_nxt_) {
          return;
        }
        rcv_buf_.insert(rcv_buf_.end(), payload.begin(), payload.end());
        rcv_nxt_ =
            seqnum::Add(rcv_nxt_, static_cast<seqnum::Size>(payload.size()));
        SendSegment(route, static_cast<uint8_t>(header::TCPFlags::kAck),
                    snd_nxt_, rcv_nxt_, {});
      }
      break;
    }
    case TcpState::kCloseWait:
      break;
    case TcpState::kLastAck: {
      if (header::HasFlag(flags, header::TCPFlags::kAck)) {
        state_ = TcpState::kClosed;
        UnregisterConnected();
        if (owns_port_) {
          stack_->ReleasePort(header::kIPv4ProtocolNumber,
                              header::kTCPProtocolNumber, local_port_);
        }
      }
      break;
    }
    default:
      break;
  }
}

}  // namespace netstack::transport::tcp
