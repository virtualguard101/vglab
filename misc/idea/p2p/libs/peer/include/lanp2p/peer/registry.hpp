#pragma once

#include "lanp2p/common/types.hpp"

#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace lanp2p::peer {

class Registry {
public:
  void upsert(PeerInfo peer);
  void expire(std::chrono::seconds ttl);
  std::vector<PeerInfo> list() const;
  std::optional<PeerInfo> find_by_id(const std::string& node_id) const;
  std::optional<PeerInfo> find_by_address(const std::string& address) const;

private:
  mutable std::mutex mutex_;
  std::unordered_map<std::string, PeerInfo> peers_;
};

} // namespace lanp2p::peer
