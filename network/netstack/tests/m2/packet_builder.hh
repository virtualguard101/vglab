/**
 * @file packet_builder.hh
 * @brief M2 测试用 IPv4/TCP 造包与解析辅助。
 *
 * ## 与 M1 `MakeIpv4Udp` 对称
 *
 * 测试帧格式：**无以太网头**，直接是完整 IPv4 datagram。
 *
 * @code
 * [ IPv4 头 20B | TCP 头 20B | payload ]
 * @endcode
 *
 * - 入站：`InjectInbound(kIPv4ProtocolNumber, PacketBuffer(MakeIpv4Tcp(...)))`
 * - 出站：`DrainOutbound()` 后用 `ParseIpv4Tcp` 断言 SYN-ACK / 数据 / FIN
 *
 * M2 测试里 TCP checksum 写 0（入站不验，见 ADR 004）。
 *
 * ## 造包与 RFC 793 事件对应
 *
 * | `TcpSegmentFields.flags` | 模拟事件 |
 * |--------------------------|----------|
 * | kSyn | LISTEN 上 RCV SYN |
 * | kAck | SYN-RECEIVED 上 RCV ACK |
 * | kAck\|kPsh + payload | ESTABLISHED 上 RCV 数据 |
 * | kFin\|kAck | ESTABLISHED 上 RCV FIN |
 *
 * @see docs/tcp-rfc793-states.md
 * @see tests/m1/udp_echo_test.cc
 * @see docs/m2.md
 */

#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "netstack/address.hh"
#include "netstack/header/ipv4.hh"
#include "netstack/header/tcp.hh"

namespace netstack::test::m2 {

/** @brief 构造 TCP 段时只需填 flags / seq / ack（M2 无选项）。 */
struct TcpSegmentFields {
  uint8_t flags{};
  uint32_t seq{};
  uint32_t ack{};
};

/**
 * @brief 构造完整 IPv4 datagram（头 + TCP 头 + 可选 payload）。
 *
 * @param sport 客户端源端口（入站时写在 TCP 源端口字段）
 * @param dport 服务端目的端口（Listen 端口）
 */
inline std::vector<uint8_t> MakeIpv4Tcp(netstack::IPv4Address src,
                                        netstack::IPv4Address dst,
                                        uint16_t sport, uint16_t dport,
                                        TcpSegmentFields tcp,
                                        std::span<const uint8_t> payload = {}) {
  const uint16_t tcp_len =
      static_cast<uint16_t>(netstack::header::kTCPMinimumSize + payload.size());
  const uint16_t total_len =
      static_cast<uint16_t>(netstack::header::kIPv4MinimumSize + tcp_len);

  std::vector<uint8_t> buf(total_len, 0);
  const size_t tcp_off = netstack::header::kIPv4MinimumSize;

  netstack::header::TCPHeader hdr(
      std::span<uint8_t>(buf.data() + tcp_off, tcp_len));
  netstack::header::TCPFields fields{};
  fields.src_port = sport;
  fields.dst_port = dport;
  fields.seq_num = tcp.seq;
  fields.ack_num = tcp.ack;
  fields.data_offset = netstack::header::kTCPMinimumSize;
  fields.flags = tcp.flags;
  fields.window_size = 65535;
  fields.checksum = 0;
  hdr.Encode(fields);
  std::copy(payload.begin(), payload.end(),
            buf.begin() + tcp_off + netstack::header::kTCPMinimumSize);

  netstack::header::IPv4Header ip(std::span<uint8_t>(buf.data(), buf.size()));
  netstack::header::IPv4Fields ip_fields{};
  ip_fields.ihl = netstack::header::kIPv4MinimumSize;
  ip_fields.total_length = total_len;
  ip_fields.ttl = 64;
  ip_fields.protocol = netstack::header::kTCPProtocolNumber;
  ip_fields.src_addr = src;
  ip_fields.dst_addr = dst;
  ip.Encode(ip_fields);
  ip.SetChecksumField(0);
  ip.SetChecksumField(static_cast<uint16_t>(~ip.CalculateChecksum()));

  return buf;
}

/** @brief 从出站 IPv4 帧解析 TCP 关键字段与载荷。 */
struct ParsedTcp {
  netstack::IPv4Address dst{};
  uint16_t dport{};
  uint8_t flags{};
  uint32_t seq{};
  uint32_t ack{};
  std::string payload;
};

inline bool ParseIpv4Tcp(const std::vector<uint8_t>& frame, ParsedTcp& out) {
  if (frame.size() <
      netstack::header::kIPv4MinimumSize + netstack::header::kTCPMinimumSize) {
    return false;
  }
  netstack::header::IPv4Header ip(
      std::span<uint8_t>(const_cast<uint8_t*>(frame.data()), frame.size()));
  if (!ip.IsValid(frame.size()) || !ip.IsChecksumValid()) {
    return false;
  }
  out.dst = ip.DestinationAddress();
  const auto hlen = ip.HeaderLength();
  netstack::header::TCPHeader tcp(std::span<uint8_t>(
      const_cast<uint8_t*>(frame.data()) + hlen, frame.size() - hlen));
  if (!tcp.IsValid(frame.size() - hlen)) {
    return false;
  }
  out.dport = tcp.DestinationPort();
  out.flags = tcp.Flags();
  out.seq = tcp.SequenceNumber();
  out.ack = tcp.AcknowledgmentNumber();
  const auto pl = tcp.Payload();
  out.payload.assign(pl.begin(), pl.end());
  return true;
}

}  // namespace netstack::test::m2
