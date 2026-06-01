#include "lanp2p/cli/commands.hpp"

#include "lanp2p/cli/args.hpp"
#include "lanp2p/core/node.hpp"
#include "lanp2p/index/file_index.hpp"
#include "lanp2p/transfer/client.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <thread>

namespace lanp2p::cli {
namespace {

core::Node* g_node = nullptr;
std::atomic<bool> g_stop{false};

void on_signal(int) {
  g_stop = true;
  if (g_node) g_node->request_stop();
}

std::string flag_value(int argc, char** argv, const std::string& flag, const std::string& def = {}) {
  for (int i = 1; i + 1 < argc; ++i) {
    if (flag == argv[i]) return argv[i + 1];
  }
  return def;
}

} // namespace

int run_command(int argc, char** argv, const ParsedArgs& args) {
  const auto& cmd = args.command;
  const auto& cfg = args.config;

  if (cmd == "run") {
    core::Node node(cfg);
    g_node = &node;
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    node.run_daemon();
    std::cout << "lanp2p running (Ctrl+C to stop)\n"
              << "  share: " << cfg.share_dir << '\n'
              << "  tcp: " << cfg.tcp_port << "  udp: " << cfg.udp_port << '\n';

    while (!g_stop) {
      std::this_thread::sleep_for(std::chrono::seconds(2));
      const auto peer_count = node.peers().list().size();
      std::cout << "\r peers online: " << peer_count << std::flush;
    }
    std::cout << '\n';
    return 0;
  }

  if (cmd == "scan" || cmd == "peers") {
    core::Node node(cfg);
    node.run_discovery_only();
    std::this_thread::sleep_for(std::chrono::seconds(args.wait_seconds));
    const auto peers = node.peers().list();
    node.request_stop();

    if (peers.empty()) {
      std::cout << "No peers found.\n";
      return 0;
    }
    for (const auto& p : peers) {
      std::cout << p.address << ':' << p.tcp_port << "  id=" << p.node_id << "  host=" << p.hostname
                << '\n';
    }
    return 0;
  }

  if (cmd == "local-list") {
    index::FileIndex index(cfg.share_dir);
    index.refresh();
    for (const auto& f : index.entries()) {
      std::cout << f.relative_path << "  (" << f.size << " bytes)\n";
    }
    return 0;
  }

  if (cmd == "list") {
    const auto host = flag_value(argc, argv, "--host");
    if (host.empty()) {
      print_usage(argv[0]);
      return 1;
    }
    const auto port = parse_port(flag_value(argc, argv, "--port", std::to_string(cfg.tcp_port)),
                                 cfg.tcp_port);

    boost::asio::io_context io;
    const auto files = transfer::list_remote_files(io, host, port);
    if (files.empty()) {
      std::cerr << "No files or failed to connect.\n";
      return 1;
    }
    for (const auto& f : files) {
      std::cout << f.relative_path << "  (" << f.size << " bytes)\n";
    }
    return 0;
  }

  if (cmd == "download") {
    const auto host = flag_value(argc, argv, "--host");
    const auto remote = flag_value(argc, argv, "--remote");
    if (host.empty() || remote.empty()) {
      print_usage(argv[0]);
      return 1;
    }
    const auto port = parse_port(flag_value(argc, argv, "--port", std::to_string(cfg.tcp_port)),
                                 cfg.tcp_port);
    std::filesystem::path local = flag_value(argc, argv, "--local", "");
    if (local.empty()) {
      local = cfg.download_dir / std::filesystem::path(remote).filename();
    }

    boost::asio::io_context io;
    const auto started = std::chrono::steady_clock::now();
    const bool ok = transfer::download_file(
        io, host, port, remote, local,
        [](const DownloadProgress& p) {
          if (p.total == 0) return;
          const int pct = static_cast<int>((100.0 * p.received) / p.total);
          std::cout << "\r " << pct << "%  " << p.received << '/' << p.total << std::flush;
        });
    std::cout << '\n';
    if (!ok) {
      std::cerr << "Download failed.\n";
      return 1;
    }
    const auto elapsed =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
    std::error_code ec;
    const auto bytes = std::filesystem::file_size(local, ec);
    if (!ec && elapsed > 0) {
      const double mib_s = (bytes / (1024.0 * 1024.0)) / elapsed;
      std::cout << "Saved to " << local << " (" << bytes << " bytes, " << mib_s << " MiB/s)\n";
    } else {
      std::cout << "Saved to " << local << '\n';
    }
    return 0;
  }

  print_usage(argv[0]);
  return 1;
}

} // namespace lanp2p::cli
