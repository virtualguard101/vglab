#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>

/**
 * @file defaults.hpp
 * @brief Default constants used across lanp2p modules.
 *
 * These values act as tunable defaults for ports, timeouts, and transfer
 * behavior. The CLI may override them via flags.
 */
namespace lanp2p {

/// Default UDP port for LAN discovery.
inline constexpr uint16_t kDefaultUdpPort = 55555;
/// Default TCP port for the transfer server.
inline constexpr uint16_t kDefaultTcpPort = 55556;
/// Default chunk size used when streaming file contents over TCP.
inline constexpr std::size_t kTransferChunkSize = 1024 * 1024;  // 1 MiB
/// Default time-to-live for peers in the registry.
inline constexpr std::chrono::seconds kPeerTtl{30};
/// Default interval for periodic maintenance tasks.
inline constexpr std::chrono::seconds kMaintenanceInterval{5};
/// Default interval between periodic discovery broadcasts.
inline constexpr std::chrono::seconds kDiscoveryBroadcastInterval{3};

}  // namespace lanp2p
