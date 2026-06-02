#pragma once

#include <filesystem>
#include <string>

/**
 * @file config.hpp
 * @brief Runtime configuration for a lanp2p node and CLI.
 *
 * This header is kept minimal and depends only on the standard library. It
 * describes user-facing configuration knobs such as ports and directories.
 */
namespace lanp2p {

/**
 * @brief Node and CLI runtime configuration.
 *
 * Values are intended to be provided by CLI arguments with sensible defaults.
 * `node_id` and `hostname` may be auto-filled by `common/identity`.
 */
struct AppConfig {
  /// UDP port used for LAN discovery (HELLO/HELLO_ACK).
  uint16_t udp_port{55555};
  /// TCP port used for the file transfer server.
  uint16_t tcp_port{55556};
  /// Local directory shared to peers (files are listed relative to this root).
  std::filesystem::path share_dir{"./share"};
  /// Local directory where downloads are written.
  std::filesystem::path download_dir{"./downloads"};
  /// Local node identifier announced to peers.
  std::string node_id;
  /// Local hostname announced to peers (best-effort, display only).
  std::string hostname;
};

}  // namespace lanp2p
