/**
 * @file address.cc
 * @brief IPv4Address / LinkAddress 的实现（netstack 核心模块）。
 *
 * 本模块处理「人类可读字符串」与「报文线格式字节」之间的转换，
 * 不涉及 IP 头字段偏移（那属于 header/ipv4）。
 *
 * @see include/netstack/address.hh
 */

#include "netstack/address.hh"

#include <charconv>
#include <iomanip>
#include <sstream>
#include <string>

namespace netstack {

/**
 * @brief 解析点分十进制 IPv4，如 "192.168.1.1"。
 *
 * 使用 C++17 std::from_chars 避免异常；每段须完整消耗且 ≤255。
 */
std::optional<IPv4Address> IPv4Address::Parse(std::string_view str) {
  IPv4Address addr{};
  size_t start = 0;
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

/** @brief 从 IPv4 头内 4 字节读取（网络序 = 报文顺序）。*/
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

/** @brief MAC 格式化输出，与 references/tcpip.LinkAddress.String 一致。*/
std::string LinkAddress::ToString() const {
  // Match the reference formatting: "%02x:%02x:%02x:%02x:%02x:%02x".
  std::ostringstream os;
  os << std::hex << std::nouppercase << std::setfill('0');
  for (size_t i = 0; i < octets.size(); ++i) {
    if (i != 0) {
      os << ':';
    }
    os << std::setw(2) << static_cast<unsigned>(octets[i]);
  }
  return os.str();
}

/**
 * @brief 解析 MAC：支持 ':' 或 '-' 分隔的 6 段十六进制。
 *
 * 对标 references/tcpip.ParseMACAddress。
 */
std::optional<LinkAddress> LinkAddress::ParseMACAddress(std::string_view s) {
  // Split on ':' or '-' (references/tcpip.ParseMACAddress).
  std::array<std::string_view, 6> parts{};
  size_t part_count = 0;
  size_t start = 0;

  auto flush_part = [&](size_t end) -> bool {
    if (part_count >= parts.size()) {
      return false;
    }
    parts[part_count++] = s.substr(start, end - start);
    return true;
  };

  for (size_t i = 0; i < s.size(); ++i) {
    const char c = s[i];
    if (c == ':' || c == '-') {
      if (!flush_part(i)) {
        return std::nullopt;
      }
      start = i + 1;
    }
  }
  if (!flush_part(s.size())) {
    return std::nullopt;
  }
  if (part_count != 6) {
    return std::nullopt;
  }

  LinkAddress addr{};
  for (size_t i = 0; i < 6; ++i) {
    const auto part = parts[i];
    if (part.empty() || part.size() > 2) {
      return std::nullopt;
    }
    unsigned int byte = 0;
    const auto* begin = part.data();
    const auto* end = begin + part.size();
    const auto [ptr, ec] = std::from_chars(begin, end, byte, 16);
    if (ec != std::errc{} || ptr != end || byte > 0xffu) {
      return std::nullopt;
    }
    addr.octets[i] = static_cast<uint8_t>(byte);
  }

  return addr;
}

}  // namespace netstack
