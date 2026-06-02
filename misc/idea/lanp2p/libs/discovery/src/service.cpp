#include "lanp2p/discovery/service.hpp"

#include <sys/socket.h>

#include "lanp2p/common/defaults.hpp"
#include "lanp2p/protocol/codec.hpp"

namespace lanp2p::discovery {
namespace {
using boost::asio::ip::udp;
using protocol::MsgType;
}  // namespace

Service::Service(boost::asio::io_context& io, peer::Registry& registry,
                 uint16_t listen_port, uint16_t broadcast_port,
                 uint16_t tcp_port, std::string node_id, std::string hostname)
    : io_(io),
      registry_(registry),
      socket_(io),
      broadcast_timer_(io),
      self_{tcp_port, std::move(node_id), std::move(hostname)} {
  boost::system::error_code ec;
  socket_.open(udp::v4(), ec);
  socket_.set_option(boost::asio::socket_base::broadcast(true));
  socket_.set_option(boost::asio::socket_base::reuse_address(true));
#ifdef SO_REUSEPORT
  const int on = 1;
  ::setsockopt(socket_.native_handle(), SOL_SOCKET, SO_REUSEPORT, &on,
               sizeof(on));
#endif
  socket_.bind(udp::endpoint(udp::v4(), listen_port), ec);
  if (ec) {
    throw boost::system::system_error(ec, "discovery udp bind");
  }
  broadcast_endpoint_ =
      udp::endpoint(boost::asio::ip::address_v4::broadcast(), broadcast_port);
}

void Service::start() {
  running_ = true;
  do_receive();
  schedule_broadcast();
}

void Service::stop() {
  running_ = false;
  boost::system::error_code ec;
  (void)broadcast_timer_.cancel();
  socket_.close(ec);
}

void Service::do_receive() {
  socket_.async_receive_from(
      boost::asio::buffer(recv_buffer_), recv_from_endpoint_,
      [this](boost::system::error_code ec, std::size_t n) {
        if (!running_) return;
        if (!ec && n > 0) {
          handle_datagram(n, recv_from_endpoint_);
        }
        if (running_) do_receive();
      });
}

void Service::handle_datagram(std::size_t nbytes, const udp::endpoint& from) {
  protocol::HelloPayload remote{};
  if (!protocol::decode_hello(recv_buffer_.data(), nbytes, remote)) return;
  if (remote.node_id == self_.node_id) return;

  lanp2p::PeerInfo peer;
  peer.node_id = remote.node_id;
  peer.address = from.address().to_string();
  peer.tcp_port = remote.tcp_port;
  peer.hostname = remote.hostname;
  peer.last_seen = std::chrono::steady_clock::now();
  registry_.upsert(std::move(peer));

  if (recv_buffer_[5] == static_cast<uint8_t>(MsgType::Hello)) {
    send_hello(from, MsgType::HelloAck);
  }
}

void Service::send_hello(const udp::endpoint& target, MsgType type) {
  protocol::HelloPayload payload = self_;
  auto packet = protocol::encode_hello(payload);
  if (type == MsgType::HelloAck && packet.size() > 5) {
    packet[5] = static_cast<uint8_t>(MsgType::HelloAck);
  }
  socket_.async_send_to(boost::asio::buffer(packet), target,
                        [](boost::system::error_code, std::size_t) {});
}

void Service::schedule_broadcast() {
  broadcast_timer_.expires_after(kDiscoveryBroadcastInterval);
  broadcast_timer_.async_wait([this](boost::system::error_code ec) {
    if (!running_ || ec) return;
    send_hello(broadcast_endpoint_, MsgType::Hello);
    send_hello(udp::endpoint(boost::asio::ip::make_address_v4("127.0.0.1"),
                             broadcast_endpoint_.port()),
               MsgType::Hello);
    schedule_broadcast();
  });
}

}  // namespace lanp2p::discovery
