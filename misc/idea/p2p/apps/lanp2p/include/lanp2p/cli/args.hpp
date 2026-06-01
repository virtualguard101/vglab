#pragma once

#include "lanp2p/common/config.hpp"

#include <string>

namespace lanp2p::cli {

struct ParsedArgs {
  std::string command;
  AppConfig config;
  int wait_seconds{5};
};

// Returns false when argv is invalid (caller should print usage).
bool parse(int argc, char** argv, ParsedArgs& out);
void print_usage(const char* program);

uint16_t parse_port(const std::string& s, uint16_t def);

} // namespace lanp2p::cli
