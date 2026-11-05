#!/usr/bin/env bash
# Boot the kernel and initramfs in QEMU.
# Quit QEMU with Ctrl-A, then X.
set -euo pipefail
cd "$(dirname "$0")/.."

qemu-system-x86_64 \
    -kernel src/arch/x86/boot/bzImage \
    -initrd initramfs.cpio.gz \
    -append "console=ttyS0 earlyprintk=serial devtmpfs.mount=1" \
    -nographic \
    -m 512M \
    -enable-kvm -cpu host

# If KVM is unavailable, remove "-enable-kvm -cpu host" (software emulation).
