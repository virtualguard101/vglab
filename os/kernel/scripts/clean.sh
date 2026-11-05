#!/usr/bin/env bash
# Remove every generated artifact, including the kernel source tree.
# Only scripts/ and the other tracked files are kept.
set -euo pipefail
cd "$(dirname "$0")/.."

echo "Removing kernel source, downloaded archives, and build artifacts..."

rm -rf src
rm -rf busybox-* initramfs initramfs.cpio.gz
rm -f linux-*.tar linux-*.tar.sign linux-*.tar.xz
