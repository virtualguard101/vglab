#!/usr/bin/env bash

set -euo pipefail

VERSION=7.3
MAJOR=${VERSION%%.*}

# Download the specified version of the Linux kernel and its signature file
echo "Downloading Linux kernel version ${VERSION}..."

wget "https://cdn.kernel.org/pub/linux/kernel/v${MAJOR}.x/linux-${VERSION}.tar.xz"
wget "https://cdn.kernel.org/pub/linux/kernel/v${MAJOR}.x/linux-${VERSION}.tar.sign"

# Verify the downloaded kernel using its signature file
unxz -k linux-${VERSION}.tar.xz   # keep .xz
gpg --recv-keys "$(gpg --list-packets linux-${VERSION}.tar.sign | grep -i keyid | awk '{print $NF}')"
gpg --verify linux-${VERSION}.tar.sign linux-${VERSION}.tar
