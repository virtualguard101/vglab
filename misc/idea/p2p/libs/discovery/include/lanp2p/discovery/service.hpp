#pragma once

#include "lanp2p/peer/registry.hpp"
#include "lanp2p/protocol/message.hpp"

#include <array>
#include <boost/asio.hpp>
#include <cstdint>
#include <string>

namespace lanp2p::discovery {

// UDP LAN peer discovery (broadcast + unicast HELLO / HELLO_ACK).
class Service {
public:
  // listen_port: local UDP bind (0 = ephemeral, for scan-only clients).
  // broadcast_port: destination for HELLO broadcasts.
  Service(boost::asio::io_context& io,
          peer::Registry& registry,
          uint16_t listen_port,
          uint16_t broadcast_port,
          uint16_t tcp_port,
          std::string node_id,
          std::string hostname);

  void start();
  void stop();

private:
  void do_receive();
  void handle_datagram(std::size_t nbytes, const boost::asio::ip::udp::endpoint& from);
  void send_hello(const boost::asio::ip::udp::endpoint& target, protocol::MsgType type);
  void schedule_broadcast();

  boost::asio::io_context& io_;
  peer::Registry& registry_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::steady_timer broadcast_timer_;
  boost::asio::ip::udp::endpoint broadcast_endpoint_;
  boost::asio::ip::udp::endpoint recv_from_endpoint_;
  std::array<uint8_t, 2048> recv_buffer_{};
  protocol::HelloPayload self_;
  bool running_{false};
};

} // namespace lanp2p::discovery
