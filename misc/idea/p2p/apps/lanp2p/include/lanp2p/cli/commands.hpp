#pragma once

#include "lanp2p/cli/args.hpp"

namespace lanp2p::cli {

int run_command(int argc, char** argv, const ParsedArgs& args);

} // namespace lanp2p::cli
