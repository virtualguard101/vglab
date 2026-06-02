#include "lanp2p/common/identity.hpp"

#include <unistd.h>

#include <random>
#include <sstream>

namespace lanp2p {

std::string default_hostname() {
  char buf[256]{};
  if (gethostname(buf, sizeof(buf) - 1) == 0) {
    return buf;
  }
  return "unknown";
}

std::string default_node_id() {
  static thread_local std::mt19937_64 rng{std::random_device{}()};
  std::uniform_int_distribution<uint64_t> dist;
  std::ostringstream oss;
  oss << default_hostname() << '-' << std::hex << dist(rng);
  return oss.str();
}

}  // namespace lanp2p
