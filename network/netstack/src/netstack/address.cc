/**
 * @file address.cc
 * @brief IPv4Address 的实现（netstack 核心模块）。
 */

#include "netstack/address.hh"

#include <charconv>
#include <sstream>
#include <string>

namespace netstack {

std::optional<IPv4Address> IPv4Address::Parse(std::string_view str) {
  IPv4Address addr{};
  size_t start = 0;
  // 按 '.' 拆成四段，每段解析为 0～255
  for (int i = 0; i < 4; ++i) {
    const size_t dot = str.find('.', start);
    const std::string_view part =
        (i < 3) ? str.substr(start, dot - start) : str.substr(start);
    if (i < 3 && dot == std::string_view::npos) {
      return std::nullopt;
    }
    const auto* begin = part.data();
    const auto* end = begin + part.size();
    unsigned int octet = 0;
    const auto [ptr, ec] = std::from_chars(begin, end, octet);
    if (ec != std::errc{} || ptr != end || octet > 255) {
      return std::nullopt;
    }
    addr.octets[static_cast<size_t>(i)] = static_cast<uint8_t>(octet);
    start = dot + 1;
  }
  if (start != str.size()) {
    return std::nullopt;
  }
  return addr;
}

IPv4Address IPv4Address::FromWire(const uint8_t* data) {
  IPv4Address addr{};
  for (size_t i = 0; i < 4; ++i) {
    addr.octets[i] = data[i];
  }
  return addr;
}

void IPv4Address::WriteWire(uint8_t* data) const {
  for (size_t i = 0; i < 4; ++i) {
    data[i] = octets[i];
  }
}

std::string IPv4Address::ToString() const {
  std::ostringstream os;
  os << static_cast<unsigned>(octets[0]) << '.'
     << static_cast<unsigned>(octets[1]) << '.'
     << static_cast<unsigned>(octets[2]) << '.'
     << static_cast<unsigned>(octets[3]);
  return os.str();
}

}  // namespace netstack
