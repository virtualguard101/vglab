#pragma once

#include <boost/asio.hpp>
#include <cstdint>

#include "lanp2p/index/file_index.hpp"

/**
 * @file server.hpp
 * @brief TCP transfer server for listing and downloading files.
 *
 * The server accepts incoming TCP connections and serves requests using the
 * LP2P framed protocol (`lanp2p::protocol`). It is constructed with a reference
 * to a `lanp2p::index::FileIndex`, which provides the set of shared files and
 * safe path resolution within the share root.
 */
namespace lanp2p::transfer {

/**
 * @brief Accepts TCP connections and serves LP2P transfer requests.
 *
 * Concurrency model:
 * - The acceptor runs on the provided `io_context`.
 * - Each session is handled by `handle_session()` (implementation decides
 *   whether to run inline, spawn a thread, etc.).
 *
 * Lifetime model:
 * - `start()` begins accepting connections.
 * - `stop()` closes the acceptor to stop new sessions.
 */
class Server {
 public:
  /**
   * @brief Construct a server bound to `tcp_port`.
   * @param io I/O context used for accept operations.
   * @param tcp_port Port to listen on.
   * @param index Reference to the file index used to answer LIST and downloads.
   */
  Server(boost::asio::io_context& io, uint16_t tcp_port,
         index::FileIndex& index);

  /**
   * @brief Start accepting connections.
   */
  void start();

  /**
   * @brief Stop accepting new connections.
   */
  void stop();

 private:
  /// Initiate an async accept operation.
  void do_accept();
  /// Serve a single connected client session.
  void handle_session(boost::asio::ip::tcp::socket socket);

  boost::asio::io_context& io_;
  index::FileIndex& index_;
  boost::asio::ip::tcp::acceptor acceptor_;
};

}  // namespace lanp2p::transfer
