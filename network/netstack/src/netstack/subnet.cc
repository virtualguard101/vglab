#include "netstack/subnet.hh"

#include <limits>
#include <sstream>

namespace netstack {

namespace {

uint32_t PrefixMask(uint8_t prefix_length) {
  if (prefix_length == 0) {
    return 0;
  }
  if (prefix_length >= 32) {
    return 0xffffffffu;
  }
  return std::numeric_limits<uint32_t>::max() << (32 - prefix_length);
}

uint32_t AddressToHostU32(IPv4Address addr) {
  return (static_cast<uint32_t>(addr.octets[0]) << 24) |
         (static_cast<uint32_t>(addr.octets[1]) << 16) |
         (static_cast<uint32_t>(addr.octets[2]) << 8) |
         static_cast<uint32_t>(addr.octets[3]);
}

}  // namespace

std::optional<Subnet> Subnet::New(IPv4Address address, uint8_t prefix_length) {
  if (prefix_length > 32) {
    return std::nullopt;
  }
  const uint32_t mask = PrefixMask(prefix_length);
  const uint32_t addr = AddressToHostU32(address);
  if ((addr & ~mask) != 0) {
    return std::nullopt;
  }
  return Subnet{address, prefix_length};
}

bool Subnet::Contains(IPv4Address addr) const {
  const uint32_t mask = PrefixMask(prefix_length);
  const uint32_t net = AddressToHostU32(address);
  const uint32_t a = AddressToHostU32(addr);
  return (a & mask) == (net & mask);
}

std::string Subnet::ToString() const {
  std::ostringstream os;
  os << address.ToString() << '/' << static_cast<unsigned>(prefix_length);
  return os.str();
}

}  // namespace netstack
