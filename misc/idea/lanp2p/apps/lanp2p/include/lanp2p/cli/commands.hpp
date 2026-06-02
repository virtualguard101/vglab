#pragma once

#include "lanp2p/cli/args.hpp"

/**
 * @file commands.hpp
 * @brief CLI command dispatch/implementation entry point.
 *
 * Command handlers in the CLI layer should avoid embedding protocol logic.
 * They orchestrate user interaction and call into `lanp2p::core` and
 * `lanp2p::transfer` APIs.
 */
namespace lanp2p::cli {

/**
 * @brief Execute the parsed command.
 *
 * @param argc Original argument count (may be used for help/usage printing).
 * @param argv Original argument vector.
 * @param args Parsed arguments (command + config).
 * @return Process exit code.
 */
int run_command(int argc, char** argv, const ParsedArgs& args);

}  // namespace lanp2p::cli
