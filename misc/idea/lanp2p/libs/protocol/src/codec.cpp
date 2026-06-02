#include "lanp2p/protocol/codec.hpp"

#include <arpa/inet.h>

#include <cstring>
#include <stdexcept>

namespace lanp2p::protocol {
namespace {

using lanp2p::FileEntry;

void append_u16(std::vector<uint8_t>& out, uint16_t v) {
  uint16_t n = htons(v);
  const auto* p = reinterpret_cast<const uint8_t*>(&n);
  out.insert(out.end(), p, p + sizeof(n));
}

void append_u32(std::vector<uint8_t>& out, uint32_t v) {
  uint32_t n = htonl(v);
  const auto* p = reinterpret_cast<const uint8_t*>(&n);
  out.insert(out.end(), p, p + sizeof(n));
}

void append_u64(std::vector<uint8_t>& out, uint64_t v) {
  uint32_t hi = htonl(static_cast<uint32_t>(v >> 32));
  uint32_t lo = htonl(static_cast<uint32_t>(v & 0xffffffffu));
  const auto* ph = reinterpret_cast<const uint8_t*>(&hi);
  const auto* pl = reinterpret_cast<const uint8_t*>(&lo);
  out.insert(out.end(), ph, ph + sizeof(hi));
  out.insert(out.end(), pl, pl + sizeof(lo));
}

bool read_u16(const uint8_t*& cur, const uint8_t* end, uint16_t& v) {
  if (static_cast<size_t>(end - cur) < sizeof(uint16_t)) return false;
  uint16_t n{};
  std::memcpy(&n, cur, sizeof(n));
  v = ntohs(n);
  cur += sizeof(uint16_t);
  return true;
}

bool read_u32(const uint8_t*& cur, const uint8_t* end, uint32_t& v) {
  if (static_cast<size_t>(end - cur) < sizeof(uint32_t)) return false;
  uint32_t n{};
  std::memcpy(&n, cur, sizeof(n));
  v = ntohl(n);
  cur += sizeof(uint32_t);
  return true;
}

bool read_u64(const uint8_t*& cur, const uint8_t* end, uint64_t& v) {
  uint32_t hi{}, lo{};
  if (!read_u32(cur, end, hi) || !read_u32(cur, end, lo)) return false;
  v = (static_cast<uint64_t>(hi) << 32) | lo;
  return true;
}

bool read_string(const uint8_t*& cur, const uint8_t* end, std::string& s) {
  uint16_t len{};
  if (!read_u16(cur, end, len)) return false;
  if (static_cast<size_t>(end - cur) < len) return false;
  s.assign(reinterpret_cast<const char*>(cur), len);
  cur += len;
  return true;
}

void append_string(std::vector<uint8_t>& out, const std::string& s) {
  if (s.size() > 0xffff)
    throw std::runtime_error("string too long for protocol");
  append_u16(out, static_cast<uint16_t>(s.size()));
  out.insert(out.end(), s.begin(), s.end());
}

}  // namespace

std::vector<uint8_t> encode_hello(const HelloPayload& hello) {
  std::vector<uint8_t> body;
  append_u16(body, hello.tcp_port);
  append_string(body, hello.node_id);
  append_string(body, hello.hostname);
  return encode_header(MsgType::Hello, 0, body);
}

bool decode_hello(const uint8_t* data, size_t len, HelloPayload& out) {
  MessageHeader hdr{};
  if (!decode_header(data, len, hdr)) return false;
  if (hdr.type != static_cast<uint8_t>(MsgType::Hello) &&
      hdr.type != static_cast<uint8_t>(MsgType::HelloAck)) {
    return false;
  }
  const uint8_t* cur = data + kHeaderSize;
  const uint8_t* end = data + len;
  if (!read_u16(cur, end, out.tcp_port)) return false;
  if (!read_string(cur, end, out.node_id)) return false;
  if (!read_string(cur, end, out.hostname)) return false;
  return cur == end;
}

std::vector<uint8_t> encode_header(MsgType type, uint32_t seq,
                                   const std::vector<uint8_t>& body) {
  std::vector<uint8_t> out;
  out.insert(out.end(), kMagic, kMagic + 4);
  out.push_back(kVersion);
  out.push_back(static_cast<uint8_t>(type));
  append_u32(out, static_cast<uint32_t>(body.size()));
  append_u32(out, seq);
  out.insert(out.end(), body.begin(), body.end());
  return out;
}

bool parse_header_prefix(const uint8_t* data, size_t len, MessageHeader& hdr) {
  if (len < kHeaderSize) return false;
  if (std::memcmp(data, kMagic, 4) != 0) return false;
  if (data[4] != kVersion) return false;
  hdr.type = data[5];
  const uint8_t* cur = data + 6;
  const uint8_t* end = data + kHeaderSize;
  uint32_t body_len{};
  if (!read_u32(cur, end, body_len) || !read_u32(cur, end, hdr.seq))
    return false;
  hdr.length = body_len;
  return true;
}

bool decode_header(const uint8_t* data, size_t len, MessageHeader& hdr) {
  if (!parse_header_prefix(data, len, hdr)) return false;
  if (len != kHeaderSize + hdr.length) return false;
  return true;
}

std::vector<uint8_t> encode_string_body(const std::string& s) {
  std::vector<uint8_t> body;
  append_string(body, s);
  return body;
}

bool decode_string_body(const uint8_t* data, size_t len, std::string& out) {
  MessageHeader hdr{};
  if (!decode_header(data, len, hdr)) return false;
  const uint8_t* cur = data + kHeaderSize;
  const uint8_t* end = data + len;
  return read_string(cur, end, out) && cur == end;
}

std::vector<uint8_t> encode_file_list(const std::vector<FileEntry>& files) {
  std::vector<uint8_t> body;
  append_u32(body, static_cast<uint32_t>(files.size()));
  for (const auto& f : files) {
    append_string(body, f.relative_path);
    append_u64(body, f.size);
  }
  return encode_header(MsgType::FileList, 0, body);
}

bool decode_file_list(const uint8_t* data, size_t len,
                      std::vector<FileEntry>& out) {
  MessageHeader hdr{};
  if (!decode_header(data, len, hdr)) return false;
  if (hdr.type != static_cast<uint8_t>(MsgType::FileList)) return false;
  const uint8_t* cur = data + kHeaderSize;
  const uint8_t* end = data + len;
  uint32_t count{};
  if (!read_u32(cur, end, count)) return false;
  out.clear();
  out.reserve(count);
  for (uint32_t i = 0; i < count; ++i) {
    FileEntry e;
    if (!read_string(cur, end, e.relative_path)) return false;
    if (!read_u64(cur, end, e.size)) return false;
    out.push_back(std::move(e));
  }
  return cur == end;
}

std::vector<uint8_t> encode_file_info(const FileInfoPayload& info) {
  std::vector<uint8_t> body;
  append_string(body, info.relative_path);
  append_u64(body, info.size);
  return encode_header(MsgType::FileInfo, 0, body);
}

bool decode_file_info(const uint8_t* data, size_t len, FileInfoPayload& out) {
  MessageHeader hdr{};
  if (!decode_header(data, len, hdr)) return false;
  if (hdr.type != static_cast<uint8_t>(MsgType::FileInfo)) return false;
  const uint8_t* cur = data + kHeaderSize;
  const uint8_t* end = data + len;
  if (!read_string(cur, end, out.relative_path)) return false;
  if (!read_u64(cur, end, out.size)) return false;
  return cur == end;
}

}  // namespace lanp2p::protocol
