/**
 * @file endpoint.cc
 * @brief TCP endpoint 状态机（M2 被动打开 + 按序收发 + FIN 关闭）。
 *
 * ## Listen 流程
 *
 * 1. `PortManager::Reserve` 防止端口冲突；
 * 2. `Stack::RegisterTransportEndpoint` 写入 demuxer 表（键 = 本地 IP +
 * 端口）；
 * 3. `state_ = LISTEN`。
 *
 * ## HandlePacket ↔ RFC 793 转移表
 *
 * 完整状态图见 **docs/tcp-rfc793-states.md**；下表为 M2 实现的子集。
 *
 * | RFC 当前态 | 事件 / 段 | 发送 | RFC 下一态 | `TcpState` |
 * |------------|-----------|------|------------|------------|
 * | LISTEN | RCV SYN | SYN\|ACK | SYN-RECEIVED | kSynReceived |
 * | SYN-RECEIVED | RCV ACK | — | ESTABLISHED | kEstablished |
 * | ESTABLISHED | RCV 数据 | ACK | ESTABLISHED | kEstablished |
 * | ESTABLISHED | RCV FIN | ACK | CLOSE-WAIT | kCloseWait |
 * | CLOSE-WAIT | （应用 Close） | FIN\|ACK | LAST-ACK | kLastAck |
 * | LAST-ACK | RCV ACK | — | CLOSED | kClosed |
 *
 * M2 **不**处理乱序、不重传；seq ≠ rcv_nxt_ 的段直接丢弃。
 *
 * ## TCP 校验和（出站）
 *
 * 伪首部（12 字节）+ TCP 段，RFC 1071 累加后取反写入 checksum 字段。
 * 入站不验证（见 ADR 004）。
 *
 * @see include/netstack/transport/tcp/endpoint.hh
 * @see docs/tcp-rfc793-states.md
 * @see docs/m2.md
 */

#include "netstack/transport/tcp/endpoint.hh"

#include <algorithm>
#include <cstring>

#include "netstack/header/checksum.hh"
#include "netstack/header/ipv4.hh"
#include "netstack/header/tcp.hh"
#include "netstack/net/ipv4/send.hh"
#include "netstack/stack/stack.hh"

namespace netstack::transport::tcp {

namespace {

IPv4Address FromStackAddress(const stack::Address& addr) {
  IPv4Address out{};
  if (addr.size() >= 4) {
    for (size_t i = 0; i < 4; ++i) {
      out.octets[i] = addr[i];
    }
  }
  return out;
}

/**
 * @brief 计算 TCP 段校验和（含 IPv4 伪首部）。
 *
 * @code
 * 伪首部：src(4) | dst(4) | 0 | proto(6) | tcp_len(2)
 * checksum = ~Checksum(伪首部 + TCP段[checksum字段=0])
 * @endcode
 */
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

Endpoint::Endpoint(stack::Stack* stack) : stack_(stack) {}

stack::StackResult Endpoint::Listen(stack::NICID nic_id, IPv4Address local_addr,
                                    uint16_t port) {
  if (stack_ == nullptr) {
    return stack::StackError{stack::ErrorCode::kInvalidEndpointState};
  }
  if (state_ != TcpState::kClosed) {
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
  // RFC 793: passive OPEN — CLOSED → LISTEN
  state_ = TcpState::kListen;
  return std::nullopt;
}

std::vector<uint8_t> Endpoint::Read() {
  std::vector<uint8_t> out;
  out.swap(rcv_buf_);
  return out;
}

stack::StackResult Endpoint::Write(std::span<const uint8_t> data) {
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

void Endpoint::Close() {
  if (stack_ == nullptr) {
    return;
  }
  if (state_ == TcpState::kEstablished) {
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
    // RFC 793: ESTABLISHED + CLOSE → FIN-WAIT-1（M2 简化为 LAST-ACK）
    state_ = TcpState::kLastAck;
  } else if (state_ == TcpState::kCloseWait) {
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
    // RFC 793: CLOSE-WAIT + CLOSE → LAST-ACK
    state_ = TcpState::kLastAck;
  }
}

void Endpoint::SendSegment(stack::Route* route, uint8_t flags, uint32_t seq,
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

/**
 * @brief 入站 TCP 段处理：按 state_ 分支驱动握手/数据/关闭。
 *
 * @param route 入站 Route（local=目的 IP，remote=源 IP，由 ipv4::HandlePacket
 * 填写）。
 * @param id demuxer 填写的四元组（remote_port = 客户端源端口）。
 */
void Endpoint::HandlePacket(stack::Route* route, stack::TransportEndpointID id,
                            stack::PacketBuffer pkt) {
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
    case TcpState::kListen: {
      // RFC 793: LISTEN + RCV SYN → SYN-RECEIVED（发 SYN|ACK）
      // 只接受「纯 SYN」；带 ACK 的 SYN 在完整栈中走同时打开，M2 丢弃
      if (!header::HasFlag(flags, header::TCPFlags::kSyn) ||
          header::HasFlag(flags, header::TCPFlags::kAck)) {
        return;
      }
      remote_port_ = id.remote_port;
      remote_addr_ = id.remote_address;
      rcv_nxt_ = seqnum::Add(seq, 1);   // SYN 占一个序号
      snd_nxt_ = seqnum::Add(iss_, 1);  // 我方 SYN 将占 iss_
      SendSegment(
          route,
          static_cast<uint8_t>(header::TCPFlags::kSyn | header::TCPFlags::kAck),
          iss_, rcv_nxt_, {});
      state_ = TcpState::kSynReceived;
      break;
    }
    case TcpState::kSynReceived: {
      // 第三次握手：ACK 必须确认我方 SYN（ack == iss_ + 1 == snd_nxt_）
      if (!header::HasFlag(flags, header::TCPFlags::kAck) || ack != snd_nxt_) {
        return;
      }
      state_ = TcpState::kEstablished;
      break;
    }
    case TcpState::kEstablished: {
      // RFC 793: ESTABLISHED + RCV FIN → CLOSE-WAIT（发 ACK）
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
      // RFC 793: ESTABLISHED + RCV 数据 → ESTABLISHED（发 ACK）
      if (!payload.empty()) {
        if (seq != rcv_nxt_) {
          return;  // M2：乱序丢弃（完整栈应缓存或 SACK）
        }
        rcv_buf_.insert(rcv_buf_.end(), payload.begin(), payload.end());
        rcv_nxt_ =
            seqnum::Add(rcv_nxt_, static_cast<seqnum::Size>(payload.size()));
        SendSegment(route, static_cast<uint8_t>(header::TCPFlags::kAck),
                    snd_nxt_, rcv_nxt_, {});
      }
      break;
    }
    case TcpState::kCloseWait: {
      // RFC 793: 等待用户 CLOSE()；对端 ACK 可忽略
      break;
    }
    case TcpState::kLastAck: {
      // RFC 793: LAST-ACK + RCV ACK → CLOSED（M2 跳过 TIME-WAIT）
      if (header::HasFlag(flags, header::TCPFlags::kAck)) {
        state_ = TcpState::kClosed;
      }
      break;
    }
    default:
      break;
  }
}

}  // namespace netstack::transport::tcp
