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
std::optional<uint16_t> CalculateChecksum(const uint8_t* header,
                                          std::size_t header_len);

/// Verify header checksum (sum of all 16-bit words including checksum ==
/// 0xFFFF).
bool VerifyChecksum(const uint8_t* header, std::size_t header_len);