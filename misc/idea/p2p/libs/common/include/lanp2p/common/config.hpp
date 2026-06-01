#pragma once

#include <filesystem>
#include <string>

namespace lanp2p {

struct AppConfig {
  uint16_t udp_port{47777};
  uint16_t tcp_port{47778};
  std::filesystem::path share_dir{"./share"};
  std::filesystem::path download_dir{"./downloads"};
  std::string node_id;
  std::string hostname;
};

} // namespace lanp2p
