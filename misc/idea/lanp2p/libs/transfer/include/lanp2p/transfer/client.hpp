#pragma once

#include <boost/asio.hpp>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

#include "lanp2p/common/types.hpp"

/**
 * @file client.hpp
 * @brief Synchronous client-side APIs for listing and downloading from a peer.
 *
 * These functions are used by the CLI to interact with a remote lanp2p node via
 * TCP using the LP2P framed protocol.
 *
 * Error handling:
 * - `list_remote_files()` returns an empty list on failure.
 * - `download_file()` returns false on any failure (connect/protocol/I/O).
 *
 * Callers may provide a `ProgressCallback` for best-effort progress reporting
 * during downloads.
 */
namespace lanp2p::transfer {

/**
 * @brief Callback invoked with download progress snapshots.
 */
using ProgressCallback = std::function<void(const DownloadProgress&)>;

/**
 * @brief Request a file listing from a remote peer.
 * @param io I/O context used for resolution and connect.
 * @param host Hostname or IP address.
 * @param port Peer TCP port.
 * @return Vector of file entries; empty on failure.
 */
std::vector<FileEntry> list_remote_files(boost::asio::io_context& io,
                                         const std::string& host,
                                         uint16_t port);

/**
 * @brief Download a remote file to a local path.
 *
 * The function performs a request/response handshake to obtain file metadata
 * and then streams file data until an EOF marker is received.
 *
 * @param io I/O context used for resolution and connect.
 * @param host Hostname or IP address.
 * @param port Peer TCP port.
 * @param remote_path Relative path within the peer's share root.
 * @param local_path Destination path on the local filesystem.
 * @param on_progress Optional callback for progress updates.
 * @return True on successful complete download; false otherwise.
 */
bool download_file(boost::asio::io_context& io, const std::string& host,
                   uint16_t port, const std::string& remote_path,
                   const std::filesystem::path& local_path,
                   ProgressCallback on_progress = {});

}  // namespace lanp2p::transfer
