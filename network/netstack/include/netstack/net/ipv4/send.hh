/**
 * @file send.hh
 * @brief IPv4 出站封装（M1：构造 IPv4 头并 WritePacket）。
 *
 * 将已拼好的 L4 字节流（如 UDP 头 + payload）前面加上 IPv4 头，
 * 通过指定 NIC 的 `LinkEndpoint` 发出。channel 测试里会进入 outbound 队列。
 *
 * @see references/tcpip/network/ipv4/ipv4.go WritePacket
 * @see docs/m1.md
 */

#pragma once

#include <vector>

#include "netstack/stack/registration.hh"

namespace netstack::stack {
class Stack;
}

namespace netstack::net::ipv4 {

/**
 * @brief 发送一个 IPv4  datagram。
 *
 * @param stack 协议栈实例（查 NIC）。
 * @param nic_id 出站接口。
 * @param route local/remote_address 为 IPv4 源/目的（stack::Address 4 字节）。
 * @param ip_protocol IPv4 头 Protocol 字段（UDP=17）。
 * @param l4_bytes 传输层及以下载荷（对 UDP 即整个 UDP 段）。
 */
stack::StackResult SendPacket(stack::Stack& stack, stack::NICID nic_id,
                              stack::Route& route, uint8_t ip_protocol,
                              std::vector<uint8_t> l4_bytes);

}  // namespace netstack::net::ipv4
