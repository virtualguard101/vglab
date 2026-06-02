#include "lanp2p/peer/registry.hpp"

#include <algorithm>

namespace lanp2p::peer {

void Registry::upsert(PeerInfo peer) {
  std::lock_guard lock(mutex_);
  peers_[peer.node_id] = std::move(peer);
}

void Registry::expire(std::chrono::seconds ttl) {
  const auto now = std::chrono::steady_clock::now();
  std::lock_guard lock(mutex_);
  for (auto it = peers_.begin(); it != peers_.end();) {
    if (now - it->second.last_seen > ttl) {
      it = peers_.erase(it);
    } else {
      ++it;
    }
  }
}

std::vector<PeerInfo> Registry::list() const {
  std::lock_guard lock(mutex_);
  std::vector<PeerInfo> out;
  out.reserve(peers_.size());
  for (const auto& [_, p] : peers_) {
    out.push_back(p);
  }
  std::sort(out.begin(), out.end(), [](const PeerInfo& a, const PeerInfo& b) {
    return a.address < b.address;
  });
  return out;
}

std::optional<PeerInfo> Registry::find_by_id(const std::string& node_id) const {
  std::lock_guard lock(mutex_);
  auto it = peers_.find(node_id);
  if (it == peers_.end()) return std::nullopt;
  return it->second;
}

std::optional<PeerInfo> Registry::find_by_address(
    const std::string& address) const {
  std::lock_guard lock(mutex_);
  for (const auto& [_, p] : peers_) {
    if (p.address == address) return p;
  }
  return std::nullopt;
}

}  // namespace lanp2p::peer
