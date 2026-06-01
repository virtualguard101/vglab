#pragma once

#include <chrono>
#include <cstdint>
#include <cstddef>

namespace lanp2p {

inline constexpr uint16_t kDefaultUdpPort = 47777;
inline constexpr uint16_t kDefaultTcpPort = 47778;
inline constexpr std::size_t kTransferChunkSize = 1024 * 1024; // 1 MiB
inline constexpr std::chrono::seconds kPeerTtl{30};
inline constexpr std::chrono::seconds kMaintenanceInterval{5};
inline constexpr std::chrono::seconds kDiscoveryBroadcastInterval{3};

} // namespace lanp2p
