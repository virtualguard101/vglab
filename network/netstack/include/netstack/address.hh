#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace netstack {

/// IPv4 address in wire (network) byte order.
/// Corresponds to references/tcpip.Address when used as 4 octets.
struct IPv4Address {
  std::array<uint8_t, 4> octets{};

  friend bool operator==(const IPv4Address& a, const IPv4Address& b) {
    return a.octets == b.octets;
  }

  /// Dotted-decimal user input (e.g. "192.168.1.1").
  static std::optional<IPv4Address> Parse(std::string_view str);

  /// Four octets from a packet buffer at \p data (network order).
  static IPv4Address FromWire(const uint8_t* data);

  void WriteWire(uint8_t* data) const;

  std::string ToString() const;
};

inline constexpr IPv4Address kIPv4Broadcast{{0xff, 0xff, 0xff, 0xff}};
inline constexpr IPv4Address kIPv4Any{{0, 0, 0, 0}};

}  // namespace netstack
