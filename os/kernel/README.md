# Linux Kernel + BusyBox + QEMU

Build a Linux kernel from source, pair it with a static BusyBox user space,
and boot the result under QEMU. Each script performs exactly one step of the
process and is run from the repository root.

The resulting kernel is a throwaway test kernel: it is never installed on the
host, and the host boot configuration is not touched.

> Target platform: x86_64 Linux. The package commands below are for Arch Linux.
> On Windows, run the scripts inside WSL2 or a Linux virtual machine.

## Build dependencies

`scripts/deps.sh` installs the complete toolchain on Arch Linux:

```bash
sudo pacman -S --needed qemu-system-x86 base-devel bc bison flex libelf \
    openssl ncurses cpio gzip wget xz bzip2 gnupg tar pahole python
```

What each group is for:

| Packages | Purpose |
| --- | --- |
| `base-devel` | gcc, make, binutils, and other host build tools |
| `bc`, `bison`, `flex`, `libelf`, `openssl`, `ncurses` | Kconfig and kernel build dependencies |
| `python`, `pahole` | kernel build scripts and BTF debug information |
| `cpio`, `gzip`, `xz`, `bzip2`, `tar` | source archives and initramfs packaging |
| `wget`, `gnupg` | kernel download and GPG signature verification |
| `qemu-system-x86` | the virtual machine used to boot the kernel |

Ubuntu/Debian equivalent:

```bash
sudo apt install qemu-system-x86 build-essential bc bison flex libelf-dev \
    libssl-dev libncurses-dev cpio gzip wget xz-utils bzip2 gnupg tar \
    dwarves python3
```

## Build and boot flow

```bash
scripts/deps.sh        # 1. install build tools and QEMU
scripts/getkernel.sh   # 2. download the kernel and verify its signature
scripts/extract.sh     # 3. unpack it into src/
scripts/config.sh      # 4. generate a QEMU-ready .config
scripts/build.sh       # 5. compile the kernel
scripts/busybox.sh     # 6. build a static BusyBox
scripts/initramfs.sh   # 7. package the initramfs
scripts/run.sh         # 8. boot it in QEMU
```

Quit QEMU with `Ctrl-A`, then `X`.

### 1. Dependencies

Install the packages listed above.

### 2. Download and verify

`getkernel.sh` downloads `linux-7.3.tar.xz` and `linux-7.3.tar.sign` from
`cdn.kernel.org`, imports the signing key referenced by the signature, and runs
`gpg --verify`. It then runs `unxz -k`, leaving both `linux-7.3.tar` and the
original `.xz`.

### 3. Extract

`extract.sh` unpacks the verified `linux-7.3.tar` into `src/` with
`--strip-components=1`, removing the top-level `linux-7.3/` directory.

### 4. Configure

`config.sh` builds the configuration in four stages:

1. **Seed** from the running kernel's `/proc/config.gz`, so the new kernel keeps
   working drivers for the host hardware.
2. **Normalize** with `make olddefconfig`, which fills in options added since
   that kernel was built.
3. **Merge** the upstream `kernel/configs/kvm_guest.config` fragment via
   `make kvm_guest.config`: virtio devices, 9p, KVM guest support, paravirtual
   clocks, and related options.
4. **Force on** the options this setup depends on, as built-in (`=y`) rather
   than modules: initramfs support, the 8250 serial console, devtmpfs, tmpfs,
   proc/sysfs, `BINFMT_SCRIPT` (for `/init`), and the virtio block/net/console
   drivers.

It also keeps DWARF/BTF debug information for GDB, disables `-Werror`, sets
`LOCALVERSION=-custom`, and disables `LOCALVERSION_AUTO` so the release string
is stable.

### 5. Compile the kernel

`build.sh` runs:

```bash
make -j"$(nproc)" bzImage
```

Only the kernel image is built; no modules are compiled. The result is
`src/arch/x86/boot/bzImage`.

### 6. Build BusyBox

`busybox.sh` downloads BusyBox 1.38.0, starts from `make defconfig`, enables
`CONFIG_STATIC=y` (the initramfs has no shared libraries), disables the `tc`
applet (it no longer compiles against current kernel headers because the CBQ
uapi was removed), and installs to `busybox-install/`.

### 7. Create the initramfs

`initramfs.sh` copies BusyBox into `initramfs/`, writes a small `/init` that
mounts proc, sysfs, and devtmpfs and then execs a shell, and packs the tree:

```bash
find . | cpio -o -H newc | gzip -9 > ../initramfs.cpio.gz
```

### 8. Boot

`run.sh` starts QEMU with the kernel and initramfs:

```bash
qemu-system-x86_64 \
    -kernel src/arch/x86/boot/bzImage \
    -initrd initramfs.cpio.gz \
    -append "console=ttyS0 earlyprintk=serial devtmpfs.mount=1" \
    -nographic -m 512M -enable-kvm -cpu host
```

`-nographic` sends the serial console to the terminal.

## Generated files

All of the following are ignored by `.gitignore`:

| Path | Contents |
| --- | --- |
| `src/` | kernel source and in-tree build output |
| `src/arch/x86/boot/bzImage` | bootable kernel image |
| `busybox-install/` | statically linked BusyBox |
| `initramfs/` | initramfs staging tree |
| `initramfs.cpio.gz` | packed initramfs |
| `linux-*.tar*` | downloaded kernel archives and signature |

To remove everything and start over, including the kernel source tree:

```bash
scripts/clean.sh
```

Only `scripts/`, `.gitignore`, and this README remain afterwards.

## Customization

- **No KVM**: remove `-enable-kvm -cpu host` from `scripts/run.sh`.
- **Interactive configuration**: uncomment `make menuconfig` at the end of
  `scripts/config.sh`.
- **Faster, smaller build**: remove the `DEBUG_INFO_*` lines in
  `scripts/config.sh`. The image gets smaller, but GDB symbols are lost.
- **GDB**: add `-s -S` to the QEMU command in `scripts/run.sh`, then in another
  terminal run `gdb src/vmlinux` and `target remote :1234`.
