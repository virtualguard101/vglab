#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "netstack/address.hh"

namespace netstack {

/// Subnet (network prefix). Corresponds to references/tcpip.Subnet.
struct Subnet {
  IPv4Address address{};
  uint8_t prefix_length{};  // 0 = mask 0.0.0.0 (/0)

  friend bool operator==(const Subnet& a, const Subnet& b) {
    return a.address == b.address && a.prefix_length == b.prefix_length;
  }

  /// Validates that \p address has only network bits set for \p prefix_length.
  static std::optional<Subnet> New(IPv4Address address, uint8_t prefix_length);

  /// True if \p addr belongs to this subnet.
  bool Contains(IPv4Address addr) const;

  std::string ToString() const;
};

/// 0.0.0.0/0 — default route destination (references/header.IPv4EmptySubnet).
inline constexpr Subnet kIPv4EmptySubnet{kIPv4Any, 0};

}  // namespace netstack
