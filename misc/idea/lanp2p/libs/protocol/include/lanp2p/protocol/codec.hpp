#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "lanp2p/common/types.hpp"
#include "lanp2p/protocol/message.hpp"

/**
 * @file codec.hpp
 * @brief Encoding/decoding helpers for the LP2P wire format.
 *
 * The codec layer is intentionally transport-agnostic. It operates on byte
 * buffers and does not depend on Boost.Asio. Higher-level modules are expected
 * to perform I/O and then invoke these functions to parse or build frames.
 *
 * Conventions:
 * - Header integers are big-endian.
 * - Functions returning `bool` indicate parse/validation success.
 * - `encode_header()` builds a complete frame (header + body) given the body.
 */
namespace lanp2p::protocol {

/**
 * @brief Encode a `HelloPayload` into a message body buffer.
 * @param hello Structured payload.
 * @return Body bytes (does not include the LP2P frame header).
 */
std::vector<uint8_t> encode_hello(const HelloPayload& hello);

/**
 * @brief Decode a `HelloPayload` from a message body buffer.
 * @param data Pointer to body bytes.
 * @param len Number of bytes available at `data`.
 * @param out Output object populated on success.
 * @return True if decoding succeeded and `out` is valid.
 */
bool decode_hello(const uint8_t* data, size_t len, HelloPayload& out);

/**
 * @brief Build a full LP2P frame from a message type, sequence, and body.
 * @param type Message type identifier.
 * @param seq Sequence number.
 * @param body Body bytes (payload).
 * @return Complete frame bytes: prefix magic/version + header fields + body.
 */
std::vector<uint8_t> encode_header(MsgType type, uint32_t seq,
                                   const std::vector<uint8_t>& body);

/**
 * @brief Decode and validate an LP2P frame header.
 *
 * Expects the input buffer to start at the magic prefix (i.e. at the beginning
 * of a frame).
 *
 * @param data Pointer to bytes starting at a potential frame boundary.
 * @param len Number of bytes available at `data`.
 * @param hdr Output header fields (type/length/seq) on success.
 * @return True if magic/version are valid and `len` is enough to parse header.
 */
bool decode_header(const uint8_t* data, size_t len, MessageHeader& hdr);

/**
 * @brief Parse header fields from a buffer that begins after magic/version.
 *
 * This helper is useful when magic/version have already been checked by the
 * caller and only the remaining header fields should be read.
 *
 * @param data Pointer to bytes starting at the type field (after
 * magic/version).
 * @param len Number of bytes available at `data`.
 * @param hdr Output header fields on success.
 * @return True if enough bytes are present and fields are decoded.
 */
bool parse_header_prefix(const uint8_t* data, size_t len, MessageHeader& hdr);

/**
 * @brief Encode a UTF-8 string as a length-prefixed body.
 * @param s String to encode.
 * @return Encoded body bytes.
 */
std::vector<uint8_t> encode_string_body(const std::string& s);

/**
 * @brief Decode a length-prefixed UTF-8 string body.
 * @param data Pointer to body bytes.
 * @param len Number of bytes available at `data`.
 * @param out Decoded string on success.
 * @return True on success.
 */
bool decode_string_body(const uint8_t* data, size_t len, std::string& out);

/**
 * @brief Encode a list of files into a body buffer.
 * @param files File entries (relative paths + sizes).
 * @return Encoded body bytes.
 */
std::vector<uint8_t> encode_file_list(const std::vector<FileEntry>& files);

/**
 * @brief Decode a list of files from a body buffer.
 * @param data Pointer to body bytes.
 * @param len Number of bytes available at `data`.
 * @param out Output list populated on success.
 * @return True on success.
 */
bool decode_file_list(const uint8_t* data, size_t len,
                      std::vector<FileEntry>& out);

/**
 * @brief Encode `FileInfoPayload` into a body buffer.
 * @param info File info payload.
 * @return Encoded body bytes.
 */
std::vector<uint8_t> encode_file_info(const FileInfoPayload& info);

/**
 * @brief Decode `FileInfoPayload` from a body buffer.
 * @param data Pointer to body bytes.
 * @param len Number of bytes available at `data`.
 * @param out Output payload populated on success.
 * @return True on success.
 */
bool decode_file_info(const uint8_t* data, size_t len, FileInfoPayload& out);

}  // namespace lanp2p::protocol
