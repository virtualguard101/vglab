# vglab

Lab repo for Computer Science self-learning. Contains codes of common scripts, teaching code, minimal verification, PoCs(Proof of Concepts), and MVPs(Minimum Viable Products) for Ideas. Mainly writen in C++, Python, Go, and Rust.

- **Lab Guidelines**：[LAB.md](LAB.md)

- **C++ LSP / clangd Troubleshooting**：[TROUBLESHOOTING.md](TROUBLESHOOTING.md)

## Getted Started

- Clone the repo with submodules

```bash
git clone --recurse-submodules git@github.com:virtualguard101/vglab.git
```

- Set up the environment

```bash
./scripts/bootstrap.sh
```

## Commands

This repo uses [Just](https://github.com/casey/just) to manage commands, see [``justfile``](justfile) for details.

Use `just --list` to list all commands.

## Scripts

### Infrastructure scripts

- Setup cuda environment: [``setup-cuda.sh``](infrastructure/scripts/setup-cuda.sh)

```bash
curl -fsSL https://raw.githubusercontent.com/virtualguard101/vglab/main/infrastructure/scripts/setup-cuda.sh | bash
```
