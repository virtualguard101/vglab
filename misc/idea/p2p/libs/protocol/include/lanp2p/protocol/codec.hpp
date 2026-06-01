#pragma once

#include "lanp2p/common/types.hpp"
#include "lanp2p/protocol/message.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace lanp2p::protocol {

std::vector<uint8_t> encode_hello(const HelloPayload& hello);
bool decode_hello(const uint8_t* data, size_t len, HelloPayload& out);

std::vector<uint8_t> encode_header(MsgType type, uint32_t seq, const std::vector<uint8_t>& body);
bool decode_header(const uint8_t* data, size_t len, MessageHeader& hdr);
bool parse_header_prefix(const uint8_t* data, size_t len, MessageHeader& hdr);

std::vector<uint8_t> encode_string_body(const std::string& s);
bool decode_string_body(const uint8_t* data, size_t len, std::string& out);

std::vector<uint8_t> encode_file_list(const std::vector<FileEntry>& files);
bool decode_file_list(const uint8_t* data, size_t len, std::vector<FileEntry>& out);

std::vector<uint8_t> encode_file_info(const FileInfoPayload& info);
bool decode_file_info(const uint8_t* data, size_t len, FileInfoPayload& out);

} // namespace lanp2p::protocol
