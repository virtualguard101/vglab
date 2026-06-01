#pragma once

#include "lanp2p/protocol/message.hpp"

#include <boost/asio.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace lanp2p::transfer {

// Framed LP2P messages over a connected TCP socket (blocking I/O).
class TcpChannel {
public:
  explicit TcpChannel(boost::asio::ip::tcp::socket socket);

  boost::asio::ip::tcp::socket& socket();

  bool read_message(std::vector<uint8_t>& out);
  bool write_raw(const uint8_t* data, std::size_t len);
  bool send_message(protocol::MsgType type, uint32_t seq, const std::vector<uint8_t>& body);
  bool send_string_message(protocol::MsgType type, const std::string& s);

private:
  bool read_all(uint8_t* data, std::size_t len);

  boost::asio::ip::tcp::socket socket_;
};

bool connect_v4(boost::asio::io_context& io,
              const std::string& host,
              uint16_t port,
              boost::asio::ip::tcp::socket& out);

} // namespace lanp2p::transfer
