#pragma once

#include <string>

#include "lanp2p/common/config.hpp"

/**
 * @file args.hpp
 * @brief CLI argument parsing for the lanp2p executable.
 *
 * The CLI layer is intentionally thin: it parses user input into an `AppConfig`
 * plus command-specific parameters, and then calls into core/transfer modules.
 */
namespace lanp2p::cli {

/**
 * @brief Result of parsing CLI arguments.
 *
 * - `command` selects a subcommand (e.g. "run", "scan", "list", "download").
 * - `config` contains runtime configuration shared by commands.
 * - `wait_seconds` is used by commands that need to wait for discovery results.
 */
struct ParsedArgs {
  std::string command;
  AppConfig config;
  int wait_seconds{5};
};

/**
 * @brief Parse CLI arguments.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @param out Output structure populated on success.
 * @return False when arguments are invalid (caller should print usage).
 */
bool parse(int argc, char** argv, ParsedArgs& out);

/**
 * @brief Print CLI usage information.
 * @param program Program name (argv[0]).
 */
void print_usage(const char* program);

/**
 * @brief Parse a port value with a default fallback.
 * @param s String to parse.
 * @param def Default value when parsing fails or is out of range.
 * @return Parsed port.
 */
uint16_t parse_port(const std::string& s, uint16_t def);

}  // namespace lanp2p::cli
