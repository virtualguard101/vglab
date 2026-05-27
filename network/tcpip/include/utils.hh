#include <cstddef>
#include <cstdint>
#include <optional>

/// Read a 16-bit network byte order (big-endian) value from a byte array.
/// @param data Pointer to the byte array.
/// @return The 16-bit value.
uint16_t ReadBE16(const uint8_t* data);

/// Read a 32-bit network byte order (big-endian) value from a byte array.
/// @param data Pointer to the byte array.
/// @return The 32-bit value.
uint32_t ReadBE32(const uint8_t* data);

/// Write a 16-bit value to a byte array in network byte order (big-endian).
/// @param data Pointer to the byte array.
/// @param value The 16-bit value to write.
/// @return Number of bytes written (2).
uint16_t WriteBE16(uint8_t* data, uint16_t value);

/// Write a 32-bit value to a byte array in network byte order (big-endian).
/// @param data Pointer to the byte array.
/// @param value The 32-bit value to write.
/// @return Number of bytes written (4).
uint32_t WriteBE32(uint8_t* data, uint32_t value);

/// RFC 791 IPv4 header checksum. Checksum field (offset 10) is treated as zero.
/// @param header Wire-format header bytes.
/// @param header_len Header length in bytes (>= 20, multiple of 4).
/// @return Ones' complement checksum, or nullopt if header_len is invalid.
std::optional<uint16_t> CalculateIPv4Checksum(const uint8_t* header,
                                              std::size_t header_len);

/// Verify header checksum (sum of all 16-bit words including checksum ==
/// 0xFFFF).
bool VerifyIPv4Checksum(const uint8_t* header, std::size_t header_len);

/// RFC 1071 TCP checksum:
/// https://datatracker.ietf.org/doc/html/rfc1071#autoid-1
///
/// TCP checksum 使用伪首部（pseudo header）:
///   - src IPv4 (32bit)
///   - dst IPv4 (32bit)
///   - zero (8bit)
///   - protocol (8bit, TCP=6)
///   - TCP length (16bit)
///
/// 然后把伪首部和 TCP 段（含校验和字段，计算时校验和视为 0）按 16bit 大端进行
/// one's complement 累加。
///
/// @param tcp_segment Wire-format TCP segment bytes (header + payload).
/// @param tcp_len TCP segment length in bytes.
/// @param src_ip Source IPv4 address, 4 bytes in network (wire) order.
/// @param dst_ip Destination IPv4 address, 4 bytes in network (wire) order.
/// @return Ones' complement checksum, or nullopt if inputs are invalid.
std::optional<uint16_t> CalculateTCPChecksum(const uint8_t* tcp_segment,
                                             std::size_t tcp_len,
                                             const uint8_t* src_ip,
                                             const uint8_t* dst_ip);

/// Verify TCP checksum (RFC 1071).
/// @param tcp_segment Wire-format TCP segment bytes (header + payload).
/// @param tcp_len TCP segment length in bytes.
/// @param src_ip Source IPv4 address, 4 bytes in network (wire) order.
/// @param dst_ip Destination IPv4 address, 4 bytes in network (wire) order.
bool VerifyTCPChecksum(const uint8_t* tcp_segment, std::size_t tcp_len,
                       const uint8_t* src_ip, const uint8_t* dst_ip);
