/**
 * @file state.hh
 * @brief TCP 连接状态（M2 子集）。
 */

#pragma once

namespace netstack::transport::tcp {

enum class TcpState {
  kClosed,
  kListen,
  kSynReceived,
  kEstablished,
  kCloseWait,
  kLastAck,
};

}  // namespace netstack::transport::tcp
