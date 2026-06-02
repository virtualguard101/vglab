#pragma once

#include <chrono>
#include <cstdint>
#include <string>

/**
 * @file types.hpp
 * @brief Domain model types shared across lanp2p modules.
 *
 * This header intentionally contains only small value types and depends on the
 * C++ standard library only, so it can be reused by low-level modules (e.g.
 * `protocol`) without pulling in networking libraries.
 */
namespace lanp2p {

/**
 * @brief A file entry exposed by a peer for listing/download.
 *
 * Paths are represented as relative, forward-slash-separated strings in the
 * protocol and CLI. Higher layers (e.g. index/transfer) are responsible for
 * resolving them safely against a configured share directory.
 */
struct FileEntry {
  /// Relative path within the share root (no leading slash).
  std::string relative_path;
  /// File size in bytes.
  uint64_t size{};
};

/**
 * @brief Information about a discovered peer on the LAN.
 *
 * Instances are typically managed by `lanp2p::peer::Registry` and updated by
 * the discovery service. `last_seen` is based on a monotonic clock.
 */
struct PeerInfo {
  /// Node identifier announced by the peer.
  std::string node_id;
  /// Dotted-decimal IPv4 address string (e.g. "192.168.1.10").
  std::string address;  // dotted IPv4
  /// TCP port where the peer's transfer server listens.
  uint16_t tcp_port{};
  /// Best-effort hostname for display purposes.
  std::string hostname;
  /// Monotonic timestamp of the last HELLO/HELLO_ACK received.
  std::chrono::steady_clock::time_point last_seen{};
};

/**
 * @brief Progress information for an ongoing download.
 */
struct DownloadProgress {
  /// Number of bytes received so far.
  uint64_t received{};
  /// Total bytes expected for the file.
  uint64_t total{};
};

}  // namespace lanp2p
