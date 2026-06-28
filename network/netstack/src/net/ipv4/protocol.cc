/**
 * @file protocol.cc
 * @brief IPv4 网络层入站处理（对标 references/tcpip/network/ipv4/ipv4.go）。
 *
 * ## 在栈中的位置
 *
 * NIC 收到以太类型 0x0800 的帧后，剥 L2（若有）并调用
 * `NetworkProtocol::HandlePacket`。本类负责 L3 校验与 demux 到传输层。
 *
 * ## HandlePacket 流水线（M0/M1）
 *
 * 1. 长度与 `IsValid` 结构检查（版本、IHL、TotalLength）；
 * 2. **显式** `IsChecksumValid()`（教学栈比参考实现更严）；
 * 3. 拒绝分片（MF 或 FragmentOffset ≠ 0）；
 * 4. 剥 IPv4 头，将 transport payload 交给 TransportDispatcher；
 * 5. 递增 packets_delivered_ 或 malformed_packets_received_。
 *
 * ## 入站 Route 语义
 *
 * 剥头前从 IPv4 头填写 route，供传输层构造四元组：
 * - `local_address` ← **目的** IP（报文发往本机）；
 * - `remote_address` ← **源** IP（对端）。
 *
 * @see include/netstack/net/ipv4/protocol.hh
 * @see docs/m0.md
 */

#include "netstack/net/ipv4/protocol.hh"

#include "netstack/header/ipv4.hh"

namespace netstack::net::ipv4 {

/** @brief 以太类型 / 网络协议号 0x0800（IPv4）。*/
stack::NetworkProtocolNumber Protocol::Number() const {
  return header::kIPv4ProtocolNumber;
}

/** @brief 无选项 IPv4 固定头 20 字节；更短包在 HandlePacket 入口丢弃。*/
int Protocol::MinimumPacketSize() const {
  return static_cast<int>(header::kIPv4MinimumSize);
}

/**
 * @brief 从仍以 IPv4 头开头的 packet 中解析源/目的地址（供路由/日志用）。
 */
void Protocol::ParseAddresses(const std::vector<uint8_t>& packet,
                              stack::Address& src, stack::Address& dst) const {
  if (packet.size() < header::kIPv4MinimumSize) {
    src.clear();
    dst.clear();
    return;
  }
  header::IPv4Header hdr(
      std::span<uint8_t>(const_cast<uint8_t*>(packet.data()), packet.size()));
  const auto s = hdr.SourceAddress();
  const auto d = hdr.DestinationAddress();
  src.assign(s.octets.begin(), s.octets.end());
  dst.assign(d.octets.begin(), d.octets.end());
}

/**
 * @brief IPv4 入站主路径；pkt.Data() 入参时以 IPv4
 * 头开始，返回前仅余传输层载荷。
 *
 * 任一步校验失败均计入 malformed_packets_received_ 并静默丢弃（教学栈与
 * Linux 行为类似：坏包不进传输层）。
 */
void Protocol::HandlePacket(stack::Route* route, stack::PacketBuffer pkt) {
  auto& data = pkt.Data();

  // --- 1. 最小长度 ---
  if (data.size() < header::kIPv4MinimumSize) {
    ++malformed_packets_received_;
    return;
  }

  header::IPv4Header ip(std::span<uint8_t>(data.data(), data.size()));

  // --- 2. 结构合法性（版本 4、IHL、TotalLength 与 buffer 关系）---
  if (!ip.IsValid(data.size())) {
    ++malformed_packets_received_;
    return;
  }

  // --- 3. 头校验和（对含 checksum 字段的整头再算，应为 0xFFFF）---
  if (!ip.IsChecksumValid()) {
    ++malformed_packets_received_;
    return;
  }

  const auto hlen = ip.HeaderLength();
  const auto tlen = ip.TotalLength();

  // TotalLength 声称的长度不得超过实际收到的字节数
  if (tlen > data.size()) {
    ++malformed_packets_received_;
    return;
  }

  // --- 4. 分片：M0/M1 不实现重组，带 MF 或非零片偏移一律丢弃 ---
  const uint8_t flags = ip.Flags();
  const uint16_t frag_off = ip.FragmentOffset();
  if ((flags & static_cast<uint8_t>(header::IPv4Flags::MoreFragments)) != 0 ||
      frag_off != 0) {
    ++malformed_packets_received_;
    return;
  }

  const auto transport_proto =
      static_cast<stack::TransportProtocolNumber>(ip.Protocol());

  // --- 5. 填写 Route（剥头前读取；local=目的 IP，remote=源 IP）---
  if (route != nullptr) {
    route->net_proto = header::kIPv4ProtocolNumber;
    const auto src = ip.SourceAddress();
    const auto dst = ip.DestinationAddress();
    route->remote_address.assign(src.octets.begin(), src.octets.end());
    route->local_address.assign(dst.octets.begin(), dst.octets.end());
  }

  // --- 6. 剥头：NetworkHeader 保留 L3 副本；Data 截到 [hlen, tlen) ---
  pkt.NetworkHeader().assign(data.begin(), data.begin() + hlen);
  data.erase(data.begin(), data.begin() + hlen);
  if (data.size() > tlen - hlen) {
    data.resize(tlen - hlen);
  }

  if (transport_dispatcher_ == nullptr) {
    return;
  }
  ++packets_delivered_;
  transport_dispatcher_->DeliverTransportPacket(route, transport_proto,
                                                std::move(pkt));
}

}  // namespace netstack::net::ipv4
