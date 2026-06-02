/**
 * @file node.hpp
 * @brief High-level orchestration entry point for running a lanp2p node.
 *
 * `lanp2p::core::Node` owns a single `boost::asio::io_context` and wires
 * together:
 * - discovery (UDP hello/ack)
 * - peer registry (TTL-based)
 * - local file index (share directory scanning)
 * - transfer server (TCP list/download handlers)
 *
 * The CLI uses this class to run either a full daemon (`run_daemon()`) or a
 * discovery-only scan mode (`run_discovery_only()`).
 */
#pragma once

#include <atomic>
#include <boost/asio.hpp>
#include <memory>
#include <thread>

#include "lanp2p/common/config.hpp"
#include "lanp2p/discovery/service.hpp"
#include "lanp2p/index/file_index.hpp"
#include "lanp2p/peer/registry.hpp"
#include "lanp2p/transfer/server.hpp"

namespace lanp2p::core {

/**
 * @brief Orchestrates discovery, indexing, and transfer on one io_context.
 *
 * Threading model:
 * - The `io_context` is driven by `io_thread_`.
 * - Public methods are expected to be called from the controlling thread
 *   (typically the CLI main thread) unless otherwise documented.
 *
 * Lifetime model:
 * - `Node` owns discovery and transfer services via unique_ptr to avoid heavy
 *   includes in the header's public API surface.
 * - `request_stop()` is cooperative; it signals the loop to stop and releases
 *   the work guard so `io_context` can exit once pending work completes.
 */
class Node {
 public:
  /**
   * @brief Construct a node with the provided configuration.
   * @param config Immutable runtime configuration (moved into the node).
   */
  explicit Node(AppConfig config);

  /**
   * @brief Run the full node daemon (discovery + transfer + maintenance).
   * @return Process exit code (0 on normal shutdown).
   */
  int run_daemon();

  /**
   * @brief Run discovery only (for `scan`/`peers` style commands).
   *
   * This mode typically binds the UDP socket to an ephemeral local port to
   * avoid competing with a concurrently running daemon instance on the same
   * host.
   *
   * @return Process exit code (0 on normal shutdown).
   */
  int run_discovery_only();

  /**
   * @brief Request the node to stop.
   *
   * The stop is cooperative; the io loop exits once pending handlers finish.
   */
  void request_stop();

  /**
   * @brief Whether a stop has been requested.
   */
  bool is_stopping() const { return stopping_.load(); }

  /**
   * @brief Access the peer registry owned by this node.
   */
  peer::Registry& peers();

  /**
   * @brief Access the local file index owned by this node.
   */
  index::FileIndex& files();

  /**
   * @brief TCP port configured for the transfer server.
   */
  uint16_t tcp_port() const { return config_.tcp_port; }

 private:
  /// Perform periodic maintenance work (expire peers, refresh index, etc.).
  void maintenance_tick();
  /// Schedule the next periodic maintenance tick on the io_context.
  void schedule_maintenance();

  /// Node runtime configuration.
  AppConfig config_;
  /// The single I/O context used by all network services.
  boost::asio::io_context io_;
  /// Keeps `io_` alive while the node is running.
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
      work_;
  /// Peer registry populated by discovery.
  peer::Registry registry_;
  /// Local index of shared files.
  index::FileIndex index_;
  /// Discovery service (UDP).
  std::unique_ptr<discovery::Service> discovery_;
  /// Transfer server (TCP).
  std::unique_ptr<transfer::Server> transfer_;
  /// True when running in discovery-only mode.
  bool discovery_only_{false};
  /// Timer used to trigger periodic maintenance work.
  boost::asio::steady_timer maintenance_timer_;
  /// Stop flag set by `request_stop()`.
  std::atomic<bool> stopping_{false};
  /// Thread that drives the io_context event loop.
  std::thread io_thread_;
};

}  // namespace lanp2p::core
