#!/usr/bin/env bash
# Compile the kernel image used by QEMU.
set -euo pipefail
cd "$(dirname "$0")/../src"

make -j"$(nproc)" bzImage

echo "Kernel image: $(pwd)/arch/x86/boot/bzImage"
