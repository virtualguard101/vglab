#include "lanp2p/cli/args.hpp"

#include <iostream>

#include "lanp2p/common/defaults.hpp"

namespace lanp2p::cli {
namespace {

std::string arg_value(int argc, char** argv, const std::string& flag,
                      const std::string& def = {}) {
  for (int i = 1; i + 1 < argc; ++i) {
    if (flag == argv[i]) return argv[i + 1];
  }
  return def;
}

}  // namespace

uint16_t parse_port(const std::string& s, uint16_t def) {
  try {
    const int p = std::stoi(s);
    if (p > 0 && p < 65536) return static_cast<uint16_t>(p);
  } catch (...) {
  }
  return def;
}

void print_usage(const char* program) {
  std::cerr << "Usage:\n"
            << "  " << program
            << " run [--share-dir DIR] [--download-dir DIR] [--tcp-port N] "
               "[--udp-port N]\n"
            << "  " << program << " scan [--wait SECONDS]\n"
            << "  " << program << " peers\n"
            << "  " << program << " list --host HOST [--port N]\n"
            << "  " << program
            << " download --host HOST --remote PATH [--local PATH] [--port N]\n"
            << "  " << program << " local-list [--share-dir DIR]\n";
}

bool parse(int argc, char** argv, ParsedArgs& out) {
  if (argc < 2) {
    print_usage(argv[0]);
    return false;
  }

  out.command = argv[1];
  out.config.share_dir = arg_value(argc, argv, "--share-dir", "./share");
  out.config.download_dir =
      arg_value(argc, argv, "--download-dir", "./downloads");
  out.config.tcp_port = parse_port(
      arg_value(argc, argv, "--tcp-port", std::to_string(kDefaultTcpPort)),
      kDefaultTcpPort);
  out.config.udp_port = parse_port(
      arg_value(argc, argv, "--udp-port", std::to_string(kDefaultUdpPort)),
      kDefaultUdpPort);

  if (out.command == "scan") {
    out.wait_seconds = std::stoi(arg_value(argc, argv, "--wait", "5"));
  } else if (out.command == "peers") {
    out.wait_seconds = std::stoi(arg_value(argc, argv, "--wait", "2"));
  }

  return true;
}

}  // namespace lanp2p::cli
