#include "ip.hh"

#include <charconv>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "utils.hh"

namespace {

bool ParseOctet(std::string_view part, uint8_t& out) {
  if (part.empty() || part.size() > 3) {
    return false;
  }
  if (part.size() > 1 && part.front() == '0') {
    return false;
  }

  int value = 0;
  const auto* begin = part.data();
  const auto* end = part.data() + part.size();
  const auto [ptr, ec] = std::from_chars(begin, end, value);
  if (ec != std::errc{} || ptr != end || value < 0 || value > 255) {
    return false;
  }

  out = static_cast<uint8_t>(value);
  return true;
}

}  // namespace

std::optional<IPv4Address> IPv4Address::Parse(std::string_view str) {
  if (str.empty()) {
    return std::nullopt;
  }

  IPv4Address addr{};
  size_t start = 0;

  for (int i = 0; i < 4; ++i) {
    const size_t end = (i < 3) ? str.find('.', start) : str.size();
    if (end == std::string_view::npos) {
      return std::nullopt;
    }

    const std::string_view part = str.substr(start, end - start);
    if (!ParseOctet(part, addr.octets[static_cast<size_t>(i)])) {
      return std::nullopt;
    }

    if (i < 3) {
      start = end + 1;
    } else {
      start = end;
    }
  }

  if (start != str.size()) {
    return std::nullopt;
  }

  return addr;
}

IPv4Address IPv4Address::FromWire(const uint8_t* data) {
  IPv4Address addr{};
  std::memcpy(addr.octets.data(), data, addr.octets.size());
  return addr;
}

void IPv4Address::WriteWire(uint8_t* data) const {
  std::memcpy(data, octets.data(), octets.size());
}

std::string IPv4Address::ToString() const {
  std::string s;
  s.reserve(15);
  for (size_t i = 0; i < octets.size(); ++i) {
    if (i > 0) {
      s.push_back('.');
    }
    s += std::to_string(octets[i]);
  }
  return s;
}

std::optional<IPv4Packet> Decoder(const uint8_t* data) {
  /// Decode header
  IPv4Header header{};
  try {
    header = {
        .version = static_cast<uint8_t>(data[0] >> 4),
        .header_len = static_cast<uint8_t>((data[0] & 0x0F) * 4),
        .type_of_service = data[1],
        .total_length = static_cast<uint16_t>(ReadBE16(data + 2)),
        .identification = static_cast<uint16_t>(ReadBE16(data + 4)),
        .flags = data[6],
        .fragment_offset = static_cast<uint16_t>(ReadBE16(data + 8) & 0x1FFF),
        .time_to_live = data[8],
        .protocol = data[9],
        .header_checksum = static_cast<uint16_t>(ReadBE16(data + 10)),
        .src_addr = IPv4Address::FromWire(data + 12),
        .dst_addr = IPv4Address::FromWire(data + 16),
    };
    header.options.assign(data + 20, data + header.header_len);
  } catch (const std::exception& e) {
    std::cerr << "Error decoding IPv4 header: " << e.what() << '\n';
    return std::nullopt;
  }

  // Verify checksum
  if (!VerifyIPv4Checksum(data, header.header_len)) {
    std::cerr << "Invalid IPv4 header checksum\n";
    return std::nullopt;
  }

  /// Assemble packet and return
  try {
    IPv4Packet packet{
        .header = header,
        .payload = std::vector<uint8_t>(data + header.header_len,
                                        data + header.total_length),
    };
    return packet;
  } catch (const std::exception& e) {
    std::cerr << "Error assembling IPv4 packet: " << e.what() << '\n';
    return std::nullopt;
  }
}

std::optional<ByteVector> Encoder(const IPv4Packet& packet) {
  /// Encode payload
  ByteVector payload{};
  if (!packet.payload.empty()) {
    try {
      payload.assign(packet.payload.begin(), packet.payload.end());
    } catch (const std::exception& e) {
      std::cerr << "Error encoding IPv4 packet payload: " << e.what() << '\n';
      return std::nullopt;
    }
  }

  /// Encode header
  ByteVector data{};

  // Helper function to write a 16-bit value to the data vector
  auto append_be16 = [&data](uint16_t value) {
    data.push_back(static_cast<uint8_t>(value >> 8));
    data.push_back(static_cast<uint8_t>(value));
  };

  try {
    data.push_back(static_cast<uint8_t>(packet.header.version << 4) |
                   static_cast<uint8_t>(packet.header.header_len / 4));
    data.push_back(packet.header.type_of_service);
    append_be16(packet.header.total_length);
    append_be16(packet.header.identification);
    append_be16(packet.header.flags << 13 | packet.header.fragment_offset);
    data.push_back(packet.header.time_to_live);
    data.push_back(packet.header.protocol);
    append_be16(0);  // checksum placeholder (offset 10)
    data.insert(data.end(), packet.header.src_addr.octets.begin(),
                packet.header.src_addr.octets.end());
    data.insert(data.end(), packet.header.dst_addr.octets.begin(),
                packet.header.dst_addr.octets.end());
    data.insert(data.end(), packet.header.options.begin(),
                packet.header.options.end());

    // Calculate checksum
    const std::size_t header_len = data.size();
    const auto checksum = CalculateIPv4Checksum(data.data(), header_len);
    if (!checksum) {
      std::cerr << "Error calculating IPv4 header checksum\n";
      return std::nullopt;
    }
    WriteBE16(data.data() + 10, *checksum);
  } catch (const std::exception& e) {
    std::cerr << "Error encoding IPv4 packet header: " << e.what() << '\n';
    return std::nullopt;
  }

  /// Assemble raw packet data and return
  try {
    // Verify checksum (self-consistency check)
    if (!VerifyIPv4Checksum(data.data(), data.size())) {
      std::cerr << "Error verifying IPv4 header checksum\n";
      return std::nullopt;
    }
    data.insert(data.end(), payload.begin(), payload.end());
  } catch (const std::exception& e) {
    std::cerr << "Error encoding IPv4 packet payload: " << e.what() << '\n';
    return std::nullopt;
  }

  return data;
}
