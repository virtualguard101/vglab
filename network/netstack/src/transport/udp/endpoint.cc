/**
 * @file endpoint.cc
 * @brief UDP echo endpoint（M1）。
 *
 * ## Bind 流程
 *
 * 1. `PortManager::Reserve` 防止端口冲突；
 * 2. `Stack::RegisterTransportEndpoint` 写入 demuxer 表；
 * 3. 记录 nic_id / local_addr / port 供回射使用。
 *
 * ## HandlePacket（echo）
 *
 * 入参 `pkt.Data()` = [UDP 头 | payload]（IPv4 已剥）。
 * 出站构造新 UDP 段，再经 IPv4 封装；**不**实现接收队列与 Read()。
 *
 * @see include/netstack/transport/udp/endpoint.hh
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

/**
 * @brief 同步 echo：剥 UDP 载荷 → 交换端口 → SendPacket。
 *
 * @param route 入站 Route（local=目的 IP，remote=源 IP，由 ipv4::HandlePacket
 * 填写）。
 * @param id demuxer 填写的四元组（remote_port = 客户端源端口）。
 */
void Endpoint::HandlePacket(stack::Route* route, stack::TransportEndpointID id,
                            stack::PacketBuffer pkt) {
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
