#pragma once

#include "lanp2p/common/types.hpp"

#include <boost/asio.hpp>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace lanp2p::transfer {

using ProgressCallback = std::function<void(const DownloadProgress&)>;

std::vector<FileEntry> list_remote_files(boost::asio::io_context& io,
                                         const std::string& host,
                                         uint16_t port);

bool download_file(boost::asio::io_context& io,
                   const std::string& host,
                   uint16_t port,
                   const std::string& remote_path,
                   const std::filesystem::path& local_path,
                   ProgressCallback on_progress = {});

} // namespace lanp2p::transfer
