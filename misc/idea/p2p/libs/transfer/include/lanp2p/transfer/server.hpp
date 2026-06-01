#pragma once

#include "lanp2p/index/file_index.hpp"

#include <boost/asio.hpp>
#include <cstdint>
#include <memory>

namespace lanp2p::transfer {

class Server {
public:
  Server(boost::asio::io_context& io, uint16_t tcp_port, index::FileIndex& index);

  void start();
  void stop();

private:
  void do_accept();
  void handle_session(boost::asio::ip::tcp::socket socket);

  boost::asio::io_context& io_;
  index::FileIndex& index_;
  boost::asio::ip::tcp::acceptor acceptor_;
};

} // namespace lanp2p::transfer
