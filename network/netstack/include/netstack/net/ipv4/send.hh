/**
 * @file send.hh
 * @brief IPv4 出站封装（M1：构造 IPv4 头并 WritePacket）。
 */

#pragma once

#include <vector>

#include "netstack/address.hh"
#include "netstack/stack/registration.hh"

namespace netstack::stack {
class Stack;
}

namespace netstack::net::ipv4 {

/** @brief 将 L4 载荷（如 UDP 头+数据）封装为 IPv4 并通过 NIC 发出。 */
stack::StackResult SendPacket(stack::Stack& stack, stack::NICID nic_id,
                              stack::Route& route, uint8_t ip_protocol,
                              std::vector<uint8_t> l4_bytes);

}  // namespace netstack::net::ipv4
