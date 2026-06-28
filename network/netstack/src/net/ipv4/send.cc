/**
 * @file send.cc
 * @brief IPv4 出站路径。
 *
 * ## 在栈中的位置
 *
 * 传输层（UDP/TCP）组装好 L4 字节后调用 `SendPacket`；本函数加 IPv4 头并
 * 经 NIC 的 `LinkEndpoint::WritePacket` 发出。
 *
 * ## 封装步骤
 *
 * 1. total_length = 20 + len(l4_bytes)；
 * 2. 填写 IPv4Fields，计算头校验和（~CalculateChecksum）；
 * 3. 拷贝 l4 到偏移 20；
 * 4. `LinkEndpoint::WritePacket(..., kIPv4ProtocolNumber, pkt)`。
 *
 * ## Route 字段约定（出站）
 *
 * - `local_address` → IPv4 **源**地址；
 * - `remote_address` → IPv4 **目的**地址；
 * - `ip_protocol` 参数为 IP 头 Protocol 字段（17=UDP，6=TCP）。
 *
 * ## 链路差异
 *
 * - **loopback**：WritePacket 立即环回入站；
 * - **channel**：只入 outbound_ 队列（M1 测试断言）；
 * - **fdbased**：write 到 TUN fd，宿主可见（M3）。
 *
 * @see include/netstack/net/ipv4/send.hh
 * @see docs/m3.md
 */

#include "netstack/net/ipv4/send.hh"

#include <algorithm>

#include "netstack/header/ipv4.hh"
#include "netstack/stack/stack.hh"

namespace netstack::net::ipv4 {

namespace {

/** @brief 将 stack::Address（4 字节向量）转为 header 模块的 IPv4Address。*/
IPv4Address FromStackAddress(const stack::Address& addr) {
  IPv4Address out{};
  if (addr.size() >= 4) {
    for (size_t i = 0; i < 4; ++i) {
      out.octets[i] = addr[i];
    }
  }
  return out;
}

}  // namespace

/**
 * @brief 在 L4 载荷前封装 IPv4 头并写出到指定 NIC。
 *
 * 校验和：先将 checksum 字段置 0，对整头 `CalculateChecksum()`，再写入 `~sum`。
 * 失败时返回 kBadAddress（NIC 不存在）或链路层 WritePacket 错误。
 */
stack::StackResult SendPacket(stack::Stack& stack, stack::NICID nic_id,
                              stack::Route& route, uint8_t ip_protocol,
                              std::vector<uint8_t> l4_bytes) {
  auto* nic = stack.GetNIC(nic_id);
  if (nic == nullptr) {
    return stack::StackError{stack::ErrorCode::kBadAddress};
  }

  const auto src = FromStackAddress(route.local_address);
  const auto dst = FromStackAddress(route.remote_address);

  const uint16_t total_length =
      static_cast<uint16_t>(header::kIPv4MinimumSize + l4_bytes.size());
  std::vector<uint8_t> buf(total_length, 0);

  header::IPv4Header ip(std::span<uint8_t>(buf.data(), buf.size()));
  header::IPv4Fields fields{};
  fields.ihl = header::kIPv4MinimumSize;
  fields.total_length = total_length;
  fields.ttl = 64;
  fields.protocol = ip_protocol;
  fields.src_addr = src;
  fields.dst_addr = dst;
  ip.Encode(fields);
  ip.SetChecksumField(0);
  ip.SetChecksumField(static_cast<uint16_t>(~ip.CalculateChecksum()));

  std::copy(l4_bytes.begin(), l4_bytes.end(),
            buf.begin() + header::kIPv4MinimumSize);

  stack::PacketBuffer pkt(std::move(buf));
  return nic->GetLinkEndpoint().WritePacket(
      &route, nullptr, header::kIPv4ProtocolNumber, std::move(pkt));
}

}  // namespace netstack::net::ipv4
