#pragma once

#include <boost/asio.hpp>
#include <cstdint>
#include <string>
#include <vector>

#include "lanp2p/protocol/message.hpp"

/**
 * @file tcp_channel.hpp
 * @brief Blocking framed LP2P messages over a connected TCP socket.
 *
 * This module is a small convenience wrapper used by both the client and
 * server sides of the TCP transfer subsystem.
 *
 * Design notes:
 * - I/O is synchronous (blocking). Concurrency is achieved by running sessions
 *   on dedicated threads (see transfer server implementation).
 * - Messages are framed using `lanp2p::protocol` header + body conventions.
 * - Callers are responsible for higher-level request/response sequencing.
 */
namespace lanp2p::transfer {

/**
 * @brief A connected TCP channel that reads/writes full LP2P frames.
 *
 * The channel owns the socket. `read_message()` reads exactly one frame and
 * returns its raw bytes (header + body). Convenience send helpers build frames
 * using `protocol::encode_header()`.
 */
class TcpChannel {
 public:
  /**
   * @brief Construct a channel by taking ownership of a connected socket.
   * @param socket Connected TCP socket (moved into the channel).
   */
  explicit TcpChannel(boost::asio::ip::tcp::socket socket);

  /**
   * @brief Access the underlying socket.
   *
   * Exposes the socket for querying endpoints, socket options, etc.
   */
  boost::asio::ip::tcp::socket& socket();

  /**
   * @brief Read a full LP2P frame from the socket.
   * @param out Output buffer replaced with the raw frame bytes on success.
   * @return True on success; false on EOF or parse/validation failure.
   */
  bool read_message(std::vector<uint8_t>& out);

  /**
   * @brief Write exactly `len` bytes to the socket.
   * @param data Pointer to bytes to write.
   * @param len Number of bytes to write.
   * @return True on success; false on I/O error.
   */
  bool write_raw(const uint8_t* data, std::size_t len);

  /**
   * @brief Encode and send a message with an explicit body.
   * @param type Message type to send.
   * @param seq Sequence number to place into the header.
   * @param body Body bytes (payload).
   * @return True on success.
   */
  bool send_message(protocol::MsgType type, uint32_t seq,
                    const std::vector<uint8_t>& body);

  /**
   * @brief Encode and send a message whose body is a single string.
   * @param type Message type to send.
   * @param s UTF-8 string body.
   * @return True on success.
   */
  bool send_string_message(protocol::MsgType type, const std::string& s);

 private:
  /**
   * @brief Read exactly `len` bytes into the provided buffer.
   * @return True on success; false on EOF or error.
   */
  bool read_all(uint8_t* data, std::size_t len);

  boost::asio::ip::tcp::socket socket_;
};

/**
 * @brief Connect to an IPv4 host/port and return a connected socket.
 *
 * The function resolves `host` using Asio's resolver and attempts to connect
 * using the first successful endpoint.
 *
 * @param io I/O context used for resolution and connect.
 * @param host Hostname or dotted-decimal IPv4 address.
 * @param port TCP port.
 * @param out Output socket set to a connected socket on success.
 * @return True on success.
 */
bool connect_v4(boost::asio::io_context& io, const std::string& host,
                uint16_t port, boost::asio::ip::tcp::socket& out);

}  // namespace lanp2p::transfer
