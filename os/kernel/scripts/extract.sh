#!/usr/bin/env bash
# Unpack the verified kernel tarball into src/.
# getkernel.sh leaves linux-7.3.tar after decompressing the .tar.xz.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p src
tar -xf linux-7.3.tar -C src --strip-components=1
