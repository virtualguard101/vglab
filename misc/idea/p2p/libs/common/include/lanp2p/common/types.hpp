#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace lanp2p {

struct FileEntry {
  std::string relative_path;
  uint64_t size{};
};

struct PeerInfo {
  std::string node_id;
  std::string address; // dotted IPv4
  uint16_t tcp_port{};
  std::string hostname;
  std::chrono::steady_clock::time_point last_seen{};
};

struct DownloadProgress {
  uint64_t received{};
  uint64_t total{};
};

} // namespace lanp2p
