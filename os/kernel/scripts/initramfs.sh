#!/usr/bin/env bash
# Build initramfs.cpio.gz from BusyBox plus a small /init.
set -euo pipefail
cd "$(dirname "$0")/.."

rm -rf initramfs
mkdir -p initramfs/{bin,sbin,etc,proc,sys,dev,tmp}
cp -a busybox-install/. initramfs/

# /init runs as PID 1 inside the guest. The kernel mounts devtmpfs early
# because run.sh passes devtmpfs.mount=1, so /dev/console already exists.
cat > initramfs/init <<'EOF'
#!/bin/sh
mount -t proc proc /proc
mount -t sysfs sysfs /sys
mount -t devtmpfs devtmpfs /dev 2>/dev/null || true

echo "Hello from Linux $(uname -r)"
exec /bin/cttyhack /bin/sh
EOF
chmod +x initramfs/init

cd initramfs
find . | cpio -o -H newc | gzip -9 > ../initramfs.cpio.gz
