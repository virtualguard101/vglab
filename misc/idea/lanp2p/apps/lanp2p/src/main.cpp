#include "lanp2p/cli/args.hpp"
#include "lanp2p/cli/commands.hpp"

int main(int argc, char** argv) {
  lanp2p::cli::ParsedArgs args;
  if (!lanp2p::cli::parse(argc, argv, args)) {
    return 1;
  }
  return lanp2p::cli::run_command(argc, argv, args);
}
