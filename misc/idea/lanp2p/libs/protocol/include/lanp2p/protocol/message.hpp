#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

/**
 * @file message.hpp
 * @brief Wire-level message primitives for the LP2P protocol.
 *
 * This header defines the on-the-wire constants and the small POD-ish payload
 * structures used by the codec layer.
 *
 * The protocol is a framed stream:
 * - A fixed-size header (`kHeaderSize`) precedes every message body.
 * - Multi-byte integers in the header are encoded in big-endian by the codec.
 *
 * See `docs/ARCHITECTURE.md` for the protocol summary and module boundaries.
 */
namespace lanp2p::protocol {

/**
 * @brief Magic bytes that start every LP2P frame.
 *
 * The codec verifies these bytes when parsing incoming data to avoid
 * misinterpreting arbitrary streams as LP2P frames.
 */
inline constexpr char kMagic[4] = {'L', 'P', '2', 'P'};

/**
 * @brief Protocol version for the current wire format.
 *
 * Versioning is kept intentionally small for the MVP; incompatible changes
 * should bump this constant and be handled by the codec.
 */
inline constexpr uint8_t kVersion = 1;

/**
 * @brief Size in bytes of the LP2P frame header.
 *
 * Layout:
 * - magic (4)
 * - version (1)
 * - message type (1)
 * - body length (4, big-endian)
 * - sequence (4, big-endian)
 */
inline constexpr std::size_t kHeaderSize =
    14;  // magic(4) + ver(1) + type(1) + len(4) + seq(4)

/**
 * @brief Message type identifiers (1 byte) used in the frame header.
 *
 * Values are stable protocol identifiers. The codec does not impose semantics
 * beyond framing; higher layers interpret the meaning and expected body.
 */
enum class MsgType : uint8_t {
  Hello = 1,
  HelloAck = 2,
  ListFiles = 10,
  FileList = 11,
  RequestFile = 20,
  FileInfo = 21,
  Data = 22,
  EndOfFile = 23,
  Error = 255,
};

/**
 * @brief Parsed LP2P frame header (excluding magic/version).
 *
 * This structure is a convenient view of the header fields after validation of
 * the magic/version prefix.
 */
struct MessageHeader {
  /// Raw message type value (same numeric space as `MsgType`).
  uint8_t type{};
  /// Size of the message body in bytes (payload only, not including the
  /// header).
  uint32_t length{};  // payload bytes
  /// Sequence number for correlating requests/responses and ordering.
  uint32_t seq{};
};

/**
 * @brief Payload for discovery beacons and acknowledgements.
 *
 * Sent via UDP by `lanp2p_discovery` to announce presence and the TCP port for
 * the transfer service.
 */
struct HelloPayload {
  /// TCP port where the peer accepts transfer connections.
  uint16_t tcp_port{};
  /// Stable identifier for the node (human-readable, not a cryptographic ID).
  std::string node_id;
  /// Best-effort hostname for display purposes.
  std::string hostname;
};

/**
 * @brief Metadata describing a single file that can be requested.
 *
 * Used during download setup before streaming `Data` frames.
 */
struct FileInfoPayload {
  /// Relative path within the peer's shared directory (no leading slash).
  std::string relative_path;
  /// Total file size in bytes.
  uint64_t size{};
};

}  // namespace lanp2p::protocol
