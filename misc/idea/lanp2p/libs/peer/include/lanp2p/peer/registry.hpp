#pragma once

#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "lanp2p/common/types.hpp"

/**
 * @file registry.hpp
 * @brief Thread-safe peer registry with TTL-based expiration.
 *
 * The registry stores the most recent information about peers discovered on the
 * local network (via UDP discovery) and provides lookup helpers for CLI and
 * higher-level orchestration.
 */
namespace lanp2p::peer {

/**
 * @brief A simple thread-safe map of peers keyed by node id.
 *
 * Thread safety:
 * - All public methods acquire an internal mutex.
 * - Returned values are copies to avoid exposing internal references.
 *
 * Expiration:
 * - `expire()` removes peers not seen within a configurable TTL.
 */
class Registry {
 public:
  /**
   * @brief Insert or update a peer record.
   * @param peer Peer info (copied/moved into the registry).
   */
  void upsert(PeerInfo peer);

  /**
   * @brief Remove peers whose `last_seen` exceeds the provided TTL.
   * @param ttl Maximum allowed age since last seen.
   */
  void expire(std::chrono::seconds ttl);

  /**
   * @brief List all peers currently in the registry.
   * @return Snapshot list of peers.
   */
  std::vector<PeerInfo> list() const;

  /**
   * @brief Find a peer by its node id.
   * @param node_id Node id to search for.
   * @return Peer info when found.
   */
  std::optional<PeerInfo> find_by_id(const std::string& node_id) const;

  /**
   * @brief Find a peer by its network address.
   * @param address Dotted IPv4 address string.
   * @return Peer info when found.
   */
  std::optional<PeerInfo> find_by_address(const std::string& address) const;

 private:
  mutable std::mutex mutex_;
  std::unordered_map<std::string, PeerInfo> peers_;
};

}  // namespace lanp2p::peer
