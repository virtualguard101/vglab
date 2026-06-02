#include "lanp2p/transfer/client.hpp"

#include <fstream>
#include <iostream>

#include "lanp2p/protocol/codec.hpp"
#include "lanp2p/transfer/tcp_channel.hpp"

namespace lanp2p::transfer {
namespace {
using protocol::MsgType;
}  // namespace

std::vector<FileEntry> list_remote_files(boost::asio::io_context& io,
                                         const std::string& host,
                                         uint16_t port) {
  boost::asio::ip::tcp::socket socket(io);
  if (!connect_v4(io, host, port, socket)) return {};

  TcpChannel channel{std::move(socket)};
  if (!channel.send_message(MsgType::ListFiles, 0, {})) return {};

  std::vector<uint8_t> msg;
  if (!channel.read_message(msg)) return {};

  std::vector<FileEntry> files;
  if (!protocol::decode_file_list(msg.data(), msg.size(), files)) return {};
  return files;
}

bool download_file(boost::asio::io_context& io, const std::string& host,
                   uint16_t port, const std::string& remote_path,
                   const std::filesystem::path& local_path,
                   ProgressCallback on_progress) {
  boost::asio::ip::tcp::socket socket(io);
  if (!connect_v4(io, host, port, socket)) return false;

  TcpChannel channel{std::move(socket)};
  if (!channel.send_string_message(MsgType::RequestFile, remote_path))
    return false;

  std::vector<uint8_t> msg;
  if (!channel.read_message(msg)) return false;

  protocol::MessageHeader hdr{};
  if (!protocol::decode_header(msg.data(), msg.size(), hdr)) return false;
  if (static_cast<MsgType>(hdr.type) == MsgType::Error) {
    std::string err;
    protocol::decode_string_body(msg.data(), msg.size(), err);
    std::cerr << "remote error: " << err << '\n';
    return false;
  }
  if (static_cast<MsgType>(hdr.type) != MsgType::FileInfo) return false;

  protocol::FileInfoPayload info{};
  if (!protocol::decode_file_info(msg.data(), msg.size(), info)) return false;

  std::error_code ec;
  std::filesystem::create_directories(local_path.parent_path(), ec);
  std::ofstream out(local_path, std::ios::binary | std::ios::trunc);
  if (!out) return false;

  uint64_t received = 0;
  while (true) {
    if (!channel.read_message(msg)) return false;
    if (!protocol::decode_header(msg.data(), msg.size(), hdr)) return false;
    const auto type = static_cast<MsgType>(hdr.type);
    if (type == MsgType::EndOfFile) break;
    if (type == MsgType::Error) return false;
    if (type != MsgType::Data) return false;
    const uint8_t* payload = msg.data() + protocol::kHeaderSize;
    const size_t plen = hdr.length;
    out.write(reinterpret_cast<const char*>(payload),
              static_cast<std::streamsize>(plen));
    if (!out) return false;
    received += plen;
    if (on_progress) {
      on_progress(DownloadProgress{received, info.size});
    }
  }

  return received == info.size;
}

}  // namespace lanp2p::transfer
