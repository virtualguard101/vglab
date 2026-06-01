#include "lanp2p/transfer/tcp_channel.hpp"

#include "lanp2p/protocol/codec.hpp"

#include <array>
#include <cstring>

namespace lanp2p::transfer {
namespace {
using boost::asio::ip::tcp;
} // namespace

TcpChannel::TcpChannel(tcp::socket socket) : socket_(std::move(socket)) {}

tcp::socket& TcpChannel::socket() { return socket_; }

bool TcpChannel::read_all(uint8_t* data, std::size_t len) {
  boost::system::error_code ec;
  boost::asio::read(socket_, boost::asio::buffer(data, len), ec);
  return !ec;
}

bool TcpChannel::read_message(std::vector<uint8_t>& out) {
  std::array<uint8_t, protocol::kHeaderSize> hdr{};
  if (!read_all(hdr.data(), hdr.size())) return false;
  protocol::MessageHeader mh{};
  if (!protocol::parse_header_prefix(hdr.data(), hdr.size(), mh)) return false;
  out.resize(protocol::kHeaderSize + mh.length);
  std::memcpy(out.data(), hdr.data(), protocol::kHeaderSize);
  if (mh.length > 0 && !read_all(out.data() + protocol::kHeaderSize, mh.length)) return false;
  return true;
}

bool TcpChannel::write_raw(const uint8_t* data, std::size_t len) {
  boost::system::error_code ec;
  boost::asio::write(socket_, boost::asio::buffer(data, len), ec);
  return !ec;
}

bool TcpChannel::send_message(protocol::MsgType type, uint32_t seq, const std::vector<uint8_t>& body) {
  auto packet = protocol::encode_header(type, seq, body);
  return write_raw(packet.data(), packet.size());
}

bool TcpChannel::send_string_message(protocol::MsgType type, const std::string& s) {
  return send_message(type, 0, protocol::encode_string_body(s));
}

bool connect_v4(boost::asio::io_context& io,
              const std::string& host,
              uint16_t port,
              tcp::socket& out) {
  tcp::resolver resolver(io);
  boost::system::error_code ec;
  auto endpoints = resolver.resolve(tcp::v4(), host, std::to_string(port), ec);
  if (ec) return false;
  out = tcp::socket(io);
  boost::asio::connect(out, endpoints, ec);
  return !ec;
}

} // namespace lanp2p::transfer
