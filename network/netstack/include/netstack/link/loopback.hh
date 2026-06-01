/**
 * @file loopback.hh
 * @brief 环回链路 endpoint（对标 tcpip/link/loopback）。
 */

#pragma once

#include <memory>

#include "netstack/stack/registration.hh"

namespace netstack::link {

/** @brief 创建 loopback LinkEndpoint。 */
std::unique_ptr<stack::LinkEndpoint> NewLoopback();

}  // namespace netstack::link
