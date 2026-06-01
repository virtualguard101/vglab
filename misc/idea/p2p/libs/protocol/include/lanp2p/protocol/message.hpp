#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace lanp2p::protocol {

inline constexpr char kMagic[4] = {'L', 'P', '2', 'P'};
inline constexpr uint8_t kVersion = 1;
inline constexpr std::size_t kHeaderSize = 14; // magic(4) + ver(1) + type(1) + len(4) + seq(4)

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

struct MessageHeader {
  uint8_t type{};
  uint32_t length{}; // payload bytes
  uint32_t seq{};
};

struct HelloPayload {
  uint16_t tcp_port{};
  std::string node_id;
  std::string hostname;
};

struct FileInfoPayload {
  std::string relative_path;
  uint64_t size{};
};

} // namespace lanp2p::protocol
