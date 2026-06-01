/**
 * @file stack.hh
 * @brief 协议栈：管理 NIC 与网络层协议注册。
 *
 * Stack 是重写的「中枢」：对标 references/tcpip/stack.Stack。
 * M0 负责把 link → NIC → ipv4 → transport stub 串起来；M1 将在此注册 UDP。
 *
 * ## 典型构造顺序
 *
 * @code
 * Stack s;
 * s.AddNetworkProtocol(std::make_unique<ipv4::Protocol>());
 * NICID id = s.CreateNIC(NewChannel(...));
 * // 测试：InjectInbound 或 loopback WritePacket
 * @endcode
 *
 * @see docs/m0.md
 * @see docs/m1.md
 */

#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "netstack/stack/network_protocol.hh"
#include "netstack/stack/nic.hh"
#include "netstack/stack/registration.hh"

namespace netstack::stack {

/** @brief 单 NIC 接收侧统计（M0 子集，对标 stack 部分 Stats）。*/
struct NICStats {
  uint64_t rx_packets{};
  uint64_t rx_bytes{};
  uint64_t unknown_protocol_rcvd{};
  uint64_t malformed_rcvd{};
};

/** @brief 栈级统计快照（聚合 NIC + 已注册网络协议计数）。*/
struct StackStats {
  NICStats nic{};
  uint64_t ipv4_malformed_received{};
  uint64_t ipv4_packets_delivered{};
};

/**
 * @brief 协议栈实例：拥有网络协议 map 与 NIC map。
 *
 * 网络协议由 unique_ptr 持有；NIC 内仅存 NetworkProtocol* 非拥有指针。
 */
class Stack {
 public:
  Stack() = default;

  /**
   * @brief 注册网络协议（全栈一份实例，各 NIC 共享同一实现指针）。
   * @note IPv4 须在 HandlePacket 前设置 TransportDispatcher（M0
   * 测试手动设置）。
   */
  void AddNetworkProtocol(std::unique_ptr<NetworkProtocol> protocol);

  /** @brief 创建 NIC 并绑定链路 endpoint；返回新 NICID。*/
  NICID CreateNIC(std::unique_ptr<LinkEndpoint> link_ep);

  NIC* GetNIC(NICID id);
  const NIC* GetNIC(NICID id) const;

  NetworkProtocol* FindProtocol(NetworkProtocolNumber number);
  const NetworkProtocol* FindProtocol(NetworkProtocolNumber number) const;

  /** @brief 读取指定 NIC 的接收统计与 IPv4 协议计数（验收测试用）。*/
  StackStats GetStats(NICID id) const;

 private:
  std::unordered_map<NetworkProtocolNumber, std::unique_ptr<NetworkProtocol>>
      protocols_{};
  std::unordered_map<NICID, std::unique_ptr<NIC>> nics_{};
  NICID next_nic_id_{1};
};

}  // namespace netstack::stack
