#!/usr/bin/env bash
# Install the host tools needed to build the kernel, BusyBox, and run QEMU.
#
# Arch Linux:
set -euo pipefail
sudo pacman -S --needed qemu-system-x86 base-devel bc bison flex libelf \
    openssl ncurses cpio gzip wget xz bzip2 gnupg tar pahole python

# Ubuntu/Debian equivalent:
#   sudo apt install qemu-system-x86 build-essential bc bison flex libelf-dev \
#       libssl-dev libncurses-dev cpio gzip wget xz-utils bzip2 gnupg tar \
#       dwarves python3
