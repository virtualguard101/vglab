#!/usr/bin/env bash
# Download and build BusyBox as a static binary for the initramfs.
set -euo pipefail
cd "$(dirname "$0")/.."

wget -nc https://busybox.net/downloads/busybox-1.38.0.tar.bz2
tar -xf busybox-1.38.0.tar.bz2

cd busybox-1.38.0
make defconfig

# The initramfs has no shared libraries, so BusyBox must be static.
sed -i 's/^# CONFIG_STATIC is not set/CONFIG_STATIC=y/' .config

# The tc applet no longer compiles: current kernel headers dropped the CBQ uapi.
sed -i 's/^CONFIG_TC=y/# CONFIG_TC is not set/' .config

make -j"$(nproc)"
make CONFIG_PREFIX="$PWD/../busybox-install" install
