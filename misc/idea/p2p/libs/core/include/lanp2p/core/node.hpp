#pragma once

#include "lanp2p/common/config.hpp"
#include "lanp2p/discovery/service.hpp"
#include "lanp2p/index/file_index.hpp"
#include "lanp2p/peer/registry.hpp"
#include "lanp2p/transfer/server.hpp"

#include <atomic>
#include <boost/asio.hpp>
#include <memory>
#include <thread>

namespace lanp2p::core {

// Orchestrates discovery, file index, and transfer server on one io_context.
class Node {
public:
  explicit Node(AppConfig config);

  int run_daemon();
  int run_discovery_only();
  void request_stop();
  bool is_stopping() const { return stopping_.load(); }

  peer::Registry& peers();
  index::FileIndex& files();
  uint16_t tcp_port() const { return config_.tcp_port; }

private:
  void maintenance_tick();
  void schedule_maintenance();

  AppConfig config_;
  boost::asio::io_context io_;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_;
  peer::Registry registry_;
  index::FileIndex index_;
  std::unique_ptr<discovery::Service> discovery_;
  std::unique_ptr<transfer::Server> transfer_;
  bool discovery_only_{false};
  boost::asio::steady_timer maintenance_timer_;
  std::atomic<bool> stopping_{false};
  std::thread io_thread_;
};

} // namespace lanp2p::core
