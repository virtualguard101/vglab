/**
 * @file stack.hh
 * @brief 协议栈：管理 NIC 与网络层协议注册。
 */

#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "netstack/stack/network_protocol.hh"
#include "netstack/stack/nic.hh"
#include "netstack/stack/registration.hh"

namespace netstack::stack {

/** @brief 单 NIC 接收侧统计（M0 子集）。*/
struct NICStats {
  uint64_t rx_packets{};
  uint64_t rx_bytes{};
  uint64_t unknown_protocol_rcvd{};
  uint64_t malformed_rcvd{};
};

/** @brief 栈级统计快照（聚合 NIC + 已注册网络协议）。*/
struct StackStats {
  NICStats nic{};
  uint64_t ipv4_malformed_received{};
  uint64_t ipv4_packets_delivered{};
};

class Stack {
 public:
  Stack() = default;

  /** @brief 注册网络协议（全栈共享一份实例，各 NIC 使用同一指针）。*/
  void AddNetworkProtocol(std::unique_ptr<NetworkProtocol> protocol);

  NICID CreateNIC(std::unique_ptr<LinkEndpoint> link_ep);

  NIC* GetNIC(NICID id);
  const NIC* GetNIC(NICID id) const;

  NetworkProtocol* FindProtocol(NetworkProtocolNumber number);
  const NetworkProtocol* FindProtocol(NetworkProtocolNumber number) const;

  /** @brief 读取指定 NIC 的接收统计与 IPv4 协议计数。*/
  StackStats GetStats(NICID id) const;

 private:
  std::unordered_map<NetworkProtocolNumber, std::unique_ptr<NetworkProtocol>>
      protocols_{};
  std::unordered_map<NICID, std::unique_ptr<NIC>> nics_{};
  NICID next_nic_id_{1};
};

}  // namespace netstack::stack
