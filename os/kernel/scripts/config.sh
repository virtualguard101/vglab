#!/usr/bin/env bash
# Generate a QEMU-friendly .config in src/.
set -euo pipefail
cd "$(dirname "$0")/../src"

# 1. Start from the running kernel's configuration, so the build keeps working
#    drivers for the host hardware. Arch exposes it through /proc.
zcat /proc/config.gz > .config
# Ubuntu/Debian: cp "/boot/config-$(uname -r)" .config

# 2. Fill in options added since the running kernel was built.
make olddefconfig

# 3. Merge the upstream KVM guest fragment: virtio, 9p, paravirt clocks, etc.
make kvm_guest.config

# 4. Force the options this QEMU setup depends on to be built in rather than
#    modules, because build.sh only compiles bzImage and no modules.
./scripts/config \
    --enable BLK_DEV_INITRD --enable RD_GZIP \
    --enable DEVTMPFS --enable DEVTMPFS_MOUNT --enable TMPFS \
    --enable PROC_FS --enable SYSFS \
    --enable TTY --enable SERIAL_8250 --enable SERIAL_8250_CONSOLE \
    --enable EARLY_PRINTK \
    --enable BINFMT_ELF --enable BINFMT_SCRIPT --enable UNIX98_PTYS \
    --enable VIRTIO_PCI --enable VIRTIO_BLK --enable VIRTIO_NET \
    --enable VIRTIO_CONSOLE --enable HW_RANDOM_VIRTIO

# 5. Keep DWARF/BTF debug info so GDB works. Delete these lines for a much
#    faster and smaller build.
./scripts/config --disable DEBUG_INFO_NONE \
    --disable DEBUG_INFO_DWARF4 --disable DEBUG_INFO_DWARF5 \
    --enable DEBUG_INFO_DWARF_TOOLCHAIN_DEFAULT

# 6. Stable version string, and do not turn compiler warnings into errors.
./scripts/config --set-str LOCALVERSION "-custom" \
    --disable LOCALVERSION_AUTO --disable WERROR

# 7. Apply the changes.
make olddefconfig

# Optional: review or tweak the configuration interactively.
# make menuconfig
