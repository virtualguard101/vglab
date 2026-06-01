#include "lanp2p/transfer/server.hpp"

#include "lanp2p/common/defaults.hpp"
#include "lanp2p/protocol/codec.hpp"
#include "lanp2p/transfer/tcp_channel.hpp"

#include <fstream>
#include <vector>

namespace lanp2p::transfer {
namespace {
using boost::asio::ip::tcp;
using protocol::MsgType;
} // namespace

Server::Server(boost::asio::io_context& io, uint16_t tcp_port, index::FileIndex& index)
    : io_(io), index_(index), acceptor_(io, tcp::endpoint(tcp::v4(), tcp_port)) {
  acceptor_.set_option(boost::asio::socket_base::reuse_address(true));
}

void Server::start() { do_accept(); }

void Server::stop() {
  boost::system::error_code ec;
  acceptor_.close(ec);
}

void Server::do_accept() {
  auto socket = std::make_shared<tcp::socket>(io_);
  acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) {
    if (!ec) {
      tcp::socket s = std::move(*socket);
      handle_session(std::move(s));
    }
    do_accept();
  });
}

void Server::handle_session(tcp::socket socket) {
  TcpChannel channel{std::move(socket)};
  std::vector<uint8_t> msg;
  if (!channel.read_message(msg)) return;

  protocol::MessageHeader hdr{};
  if (!protocol::decode_header(msg.data(), msg.size(), hdr)) return;

  const auto type = static_cast<MsgType>(hdr.type);
  if (type == MsgType::ListFiles) {
    const auto files = index_.entries();
    auto body_pkt = protocol::encode_file_list(files);
    channel.write_raw(body_pkt.data(), body_pkt.size());
    return;
  }

  if (type == MsgType::RequestFile) {
    std::string rel;
    if (!protocol::decode_string_body(msg.data(), msg.size(), rel)) return;
    const auto path = index_.resolve(rel);
    if (!path) {
      channel.send_message(MsgType::Error, 0, protocol::encode_string_body("file not found"));
      return;
    }

    std::error_code ec;
    const auto size = std::filesystem::file_size(*path, ec);
    if (ec) {
      channel.send_message(MsgType::Error, 0, protocol::encode_string_body("stat failed"));
      return;
    }

    protocol::FileInfoPayload info{rel, size};
    auto info_pkt = protocol::encode_file_info(info);
    if (!channel.write_raw(info_pkt.data(), info_pkt.size())) return;

    std::ifstream in(*path, std::ios::binary);
    if (!in) {
      channel.send_message(MsgType::Error, 0, protocol::encode_string_body("open failed"));
      return;
    }

    std::vector<char> chunk(kTransferChunkSize);
    uint32_t seq = 0;
    while (in) {
      in.read(chunk.data(), static_cast<std::streamsize>(chunk.size()));
      const auto got = static_cast<size_t>(in.gcount());
      if (got == 0) break;
      std::vector<uint8_t> body(chunk.begin(), chunk.begin() + static_cast<ptrdiff_t>(got));
      if (!channel.send_message(MsgType::Data, seq++, body)) return;
    }
    channel.send_message(MsgType::EndOfFile, seq, {});
  }
}

} // namespace lanp2p::transfer
