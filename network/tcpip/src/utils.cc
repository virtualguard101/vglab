#include "utils.hh"

#include <cstddef>
#include <cstdint>

namespace {

bool IsValidHeaderLength(std::size_t header_len) {
  return header_len >= 20 && header_len % 4 == 0;
}

void FoldCarries(uint32_t& sum) {
  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }
}

uint32_t Sum16BitWords(const uint8_t* header, std::size_t header_len,
                       bool skip_checksum_field) {
  uint32_t sum = 0;
  for (std::size_t i = 0; i + 1 < header_len; i += 2) {
    if (skip_checksum_field && i == 10) {
      continue;
    }
    sum += ReadBE16(header + i);
  }
  FoldCarries(sum);
  return sum;
}

}  // namespace

uint16_t ReadBE16(const uint8_t* data) {
  return (static_cast<uint16_t>(data[0]) << 8) | static_cast<uint16_t>(data[1]);
}

uint32_t ReadBE32(const uint8_t* data) {
  return (static_cast<uint32_t>(data[0]) << 24) |
         (static_cast<uint32_t>(data[1]) << 16) |
         (static_cast<uint32_t>(data[2]) << 8) | static_cast<uint32_t>(data[3]);
}

uint16_t WriteBE16(uint8_t* data, uint16_t value) {
  data[0] = static_cast<uint8_t>(value >> 8);
  data[1] = static_cast<uint8_t>(value);
  return 2;
}

uint32_t WriteBE32(uint8_t* data, uint32_t value) {
  data[0] = static_cast<uint8_t>(value >> 24);
  data[1] = static_cast<uint8_t>(value >> 16);
  data[2] = static_cast<uint8_t>(value >> 8);
  data[3] = static_cast<uint8_t>(value);
  return 4;
}

std::optional<uint16_t> CalculateIPv4Checksum(const uint8_t* header,
                                              std::size_t header_len) {
  if (!IsValidHeaderLength(header_len)) {
    return std::nullopt;
  }

  const uint32_t sum =
      Sum16BitWords(header, header_len, /*skip_checksum_field=*/true);
  return static_cast<uint16_t>(~sum);
}

bool VerifyIPv4Checksum(const uint8_t* header, std::size_t header_len) {
  if (!IsValidHeaderLength(header_len)) {
    return false;
  }

  const uint32_t sum =
      Sum16BitWords(header, header_len, /*skip_checksum_field=*/false);
  return sum == 0xFFFF;
}

namespace {

constexpr std::size_t kIPv4AddrBytes = 4;
constexpr std::size_t kTcpChecksumOffsetBytes =
    16;  // TCP checksum field offset
constexpr std::size_t kMinTcpHeaderLen = 20;
constexpr uint8_t kTcpProtocol = 6;  // TCP protocol number in IPv4

uint32_t SumTCPChecksumData(const uint8_t* tcp_segment, std::size_t tcp_len,
                            const uint8_t* src_ip, const uint8_t* dst_ip,
                            bool skip_checksum_field) {
  uint32_t sum = 0;

  // pseudo header
  sum += ReadBE16(src_ip);
  sum += ReadBE16(src_ip + 2);
  sum += ReadBE16(dst_ip);
  sum += ReadBE16(dst_ip + 2);
  sum += static_cast<uint16_t>(kTcpProtocol);  // zero(8) + protocol(8)
  sum += static_cast<uint16_t>(tcp_len);       // TCP length (16-bit)

  // TCP segment
  for (std::size_t i = 0; i + 1 < tcp_len; i += 2) {
    if (skip_checksum_field && i == kTcpChecksumOffsetBytes) {
      continue;
    }
    sum += ReadBE16(tcp_segment + i);
  }
  // If odd-length, pad the last byte with a zero byte.
  if (tcp_len % 2 == 1) {
    const uint16_t last = static_cast<uint16_t>(tcp_segment[tcp_len - 1]) << 8;
    sum += last;
  }

  return sum;
}

}  // namespace

std::optional<uint16_t> CalculateTCPChecksum(const uint8_t* tcp_segment,
                                             std::size_t tcp_len,
                                             const uint8_t* src_ip,
                                             const uint8_t* dst_ip) {
  if (tcp_segment == nullptr || src_ip == nullptr || dst_ip == nullptr) {
    return std::nullopt;
  }
  if (tcp_len < kMinTcpHeaderLen) {
    return std::nullopt;
  }

  uint32_t sum = SumTCPChecksumData(tcp_segment, tcp_len, src_ip, dst_ip,
                                    /*skip_checksum_field=*/true);
  FoldCarries(sum);
  return static_cast<uint16_t>(~sum);
}

bool VerifyTCPChecksum(const uint8_t* tcp_segment, std::size_t tcp_len,
                       const uint8_t* src_ip, const uint8_t* dst_ip) {
  if (tcp_segment == nullptr || src_ip == nullptr || dst_ip == nullptr) {
    return false;
  }
  if (tcp_len < kMinTcpHeaderLen) {
    return false;
  }

  uint32_t sum = SumTCPChecksumData(tcp_segment, tcp_len, src_ip, dst_ip,
                                    /*skip_checksum_field=*/false);
  FoldCarries(sum);
  return static_cast<uint16_t>(sum) == 0xFFFF;
}
