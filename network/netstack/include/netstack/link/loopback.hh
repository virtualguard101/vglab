/**
 * @file loopback.hh
 * @brief 环回链路 endpoint（对标 tcpip/link/loopback）。
 *
 * Loopback 用于本机协议栈自测：WritePacket 不离开进程，立即作为入站包
 * 回调 NIC::DeliverNetworkPacket。M0 集成测 `stack_loopback` 使用此路径。
 *
 * @see src/link/loopback.cc
 * @see docs/m0.md
 */

#pragma once

#include <memory>

#include "netstack/stack/registration.hh"

namespace netstack::link {

/** @brief 创建 loopback LinkEndpoint（MTU 65536，无 L2 头）。 */
std::unique_ptr<stack::LinkEndpoint> NewLoopback();

}  // namespace netstack::link
