#pragma once

#include <array>
#include <boost/asio.hpp>
#include <cstdint>
#include <string>

#include "lanp2p/peer/registry.hpp"
#include "lanp2p/protocol/message.hpp"

/**
 * @file service.hpp
 * @brief UDP LAN peer discovery (broadcast + unicast HELLO / HELLO_ACK).
 *
 * The discovery service periodically broadcasts a HELLO datagram on the LAN and
 * listens for incoming HELLO/HELLO_ACK messages. When a peer is observed, it
 * updates a shared `lanp2p::peer::Registry` with last-seen timestamps and the
 * peer's TCP transfer port.
 *
 * Notes:
 * - This is designed for trusted LAN environments only (no authentication).
 * - Message encoding/decoding is performed via `lanp2p::protocol`.
 */
namespace lanp2p::discovery {

/**
 * @brief Periodic UDP HELLO broadcaster and receiver.
 *
 * Socket model:
 * - Binds a UDP socket to `listen_port` (0 means ephemeral).
 * - Broadcasts HELLO to `broadcast_port` at a fixed interval.
 *
 * Registry interaction:
 * - On receiving a peer HELLO, the service upserts the peer record and replies
 *   with HELLO_ACK (unicast) to accelerate discovery.
 */
class Service {
 public:
  /**
   * @brief Construct a discovery service.
   *
   * @param io I/O context for socket and timers.
   * @param registry Peer registry to update.
   * @param listen_port Local UDP port to bind (0 = ephemeral for scan-only).
   * @param broadcast_port UDP port used as the destination for HELLO broadcast.
   * @param tcp_port TCP port advertised to peers for file transfer.
   * @param node_id Local node id advertised to peers.
   * @param hostname Local hostname advertised to peers.
   */
  Service(boost::asio::io_context& io, peer::Registry& registry,
          uint16_t listen_port, uint16_t broadcast_port, uint16_t tcp_port,
          std::string node_id, std::string hostname);

  /**
   * @brief Start discovery (begins receive loop and broadcast timer).
   */
  void start();

  /**
   * @brief Stop discovery and close the socket.
   */
  void stop();

 private:
  /// Begin an async receive for the next datagram.
  void do_receive();
  /// Handle one received datagram and update registry if applicable.
  void handle_datagram(std::size_t nbytes,
                       const boost::asio::ip::udp::endpoint& from);
  /// Send a HELLO/HELLO_ACK to the target endpoint.
  void send_hello(const boost::asio::ip::udp::endpoint& target,
                  protocol::MsgType type);
  /// Schedule the next periodic broadcast tick.
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

}  // namespace lanp2p::discovery
