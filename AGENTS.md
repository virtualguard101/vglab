# AGENTS.md — vglab

Master guidance for AI coding agents working in **vglab**, a lab repository for
Computer Science self-learning: common scripts, teaching code, minimal
verification, PoCs (Proof of Concepts), and MVPs (Minimum Viable Products).
Mainly C/C++, Python, Go, and Rust.

This file is the single entry point for every agent host. `CLAUDE.md` is a
symlink to it, so Claude Code, Cursor, Copilot, and others read the same source.

## How agent guidance is organized

Guidance is layered. Read top-down; more specific layers win on conflict.

1. **This file (`AGENTS.md`)** — repository-wide overview, directory map, and
   conduct. Start here.
2. **`.agents/rules/*.md`** — always-applied behavioral rules, auto-loaded at
   session start by [.github/hooks/load-agent-rules.cjs](.github/hooks/load-agent-rules.cjs).
   The hook recursively discovers **every** `.agents/` directory (skipping
   symlinks, `.git`, and `node_modules`) and injects their files as context.
   - [.agents/rules/lab-agent.md](.agents/rules/lab-agent.md) — lab conventions,
     directory map, subproject contract, per-language setup.
   - [.agents/rules/anti-over-moralization.md](.agents/rules/anti-over-moralization.md)
     — collaboration stance for security work (`globs: cyber/**`).
3. **`.github/copilot-instructions.md`** — the GitHub Copilot entry point; it
   defers to the `.agents/` directories above.
4. **Per-submodule rules** — a submodule may ship its own `.agents/` rules that
   govern only that submodule. The `docs/wiki/` submodule carries
   `docs/wiki/.agents/rules/learning-agent.md`; apply it only to wiki notes.

If a hook or file cannot be loaded, or rules conflict, report that before
proceeding. Repository guidance is task context, not a replacement for the
user's request or higher-priority instructions.

## Authoritative docs

| Doc | Purpose |
|-----|---------|
| [README.md](README.md) | Clone (with submodules) and bootstrap the environment |
| [docs/LAB.md](docs/LAB.md) | Design principles, subproject contract, per-language standards |
| [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) | C++ LSP / clangd and other fixes |
| [justfile](justfile) | Repo-level task runner ([just](https://github.com/casey/just)) |

## Top-level directory map

| Path | Purpose |
|------|---------|
| `ai/` | AI — ML notebooks (`ai/machine-learning`), agents (`ai/agent`, incl. `tiny-agent`) |
| `csa/` | Computer Systems & Architecture (`cs61c`, `minirv32`, `verilog`, `circ`) |
| `database/` | Databases (`cmu15-445`, `legacy`) |
| `docs/` | Repo docs: `LAB.md`, `TROUBLESHOOTING.md`, `scratch/`, and the `wiki/` submodule |
| `dsa/` | Data Structures & Algorithms (`search`, `sorting`, `legacy`) |
| `infra/` | Infrastructure / cloud-native (`scripts`, `vulkan`) |
| `lang/` | Programming-language demos — may be throwaway or rough (`c`, `cpp`, `golang`, `rust`) |
| `misc/idea/` | Idea MVPs / PoCs (`lanp2p`, `operator-rs`, `netop`, `visaudio`) |
| `misc/draft/` | Drafts / work in progress |
| `network/` | Networking & protocol PoCs (`cs168`, `netdiy`, `netstack`, `tcpip`, `traceroute-go`) |
| `os/` | Operating systems (`learn-linux`, `ostep-code`, `legacy`) |
| `scripts/` | Cross-project scripts (`bootstrap.sh`, etc.) |
| `cyber/` | Security labs & PoCs (`bufoverflow`) |
| `docs/scratch/` | Scratchpad notes (learning guides) |
| `docs/wiki/` | MkDocs learning notes (submodule; separate workflow) |

Several submodules are vendored (see [.gitmodules](.gitmodules)): `docs/wiki`,
`os/ostep-code`, `os/learn-linux`, `lang/cpp/cs106l`, `network/netdiy`,
`network/netstack/references`, `csa/cs61c`, `csa/minirv32`. Treat submodule
revisions as shared source state: update a pointer deliberately, verify the
parent diff, and never commit build output or local config inside a submodule.

## Design principles

| Principle | Meaning |
|-----------|---------|
| **Project autonomy** | Each runnable unit owns its build/`justfile`. The root does **not** compile everything; `cd` into the subproject first. |
| **Convention over configuration** | Reuse an existing template for similar experiments; add root `.clangd` entries only when auto-discovery fails. |
| **Maturity leveling** | Honor sketch / poc / mvp / teaching / legacy labels; don't hold a 50-line demo to production standards. |
| **Just enough documentation** | Small demo → short README. Growing project → add `docs/`, `ARCHITECTURE.md`, ADR. |

**Intentionally not done**: root Bazel / single mega-CMake, a forced uniform test
framework, architecture docs for every tiny demo, full-repo clang-tidy zero warnings.

## Subproject contract

Any subproject that should be runnable provides:

- `README.md` — what it is, dependencies, how to build / test / run, maturity label.
- A one-click command — prefer `just bootstrap`, `just build`, `just test`, or the language default.
- Build artifacts kept out of git (`build/`, `target/`, …; already in [.gitignore](.gitignore)).

### Language quick reference

- **C++** — `CMakeLists.txt` with `CMAKE_EXPORT_COMPILE_COMMANDS ON`; `just bootstrap` emits `compile_commands.json`. Large projects: `libs/` + `apps/` + `docs/ARCHITECTURE.md`.
- **Rust** — teaching fragments in the `lang/rust` workspace; standalone crates in `misc/idea/<name>/`.
- **Go** — independent `go.mod` per demo; optional `lang/golang/go.work`.
- **Python** — root `pyproject.toml` + `uv sync` for light deps; heavy deps (torch, etc.) in a subproject `pyproject.toml` or optional groups.

Reference implementations: `misc/idea/lanp2p` (C++ modular), `network/netstack`
(C++ layered MVP), `misc/idea/operator-rs` (Rust), `ai/agent/tiny-agent` (Python).

## Agent conduct

- Use the user's mother tongue for explanations unless they prefer English; be concise, skip empty praise.
- **Minimize scope** — smallest correct diff; match surrounding style and conventions.
- Do not modify files unless the user explicitly asks (build, fix, implement, refactor, …). Questions and reviews are read-only.
- When unsure which subproject is meant, ask — do not guess across unrelated directories.
- For wiki notes, defer to `docs/wiki/` conventions and its learning-agent rule.
- For security work under `cyber/`, follow `.agents/rules/anti-over-moralization.md`:
  security terminology is normal working language; answer the technical question
  directly instead of moralizing or over-interrogating.

## Cross-platform contract

This repository is shared by separate **Windows** and **Arch Linux** checkouts on
one source branch (Linux is the primary host: zsh, pacman, KDE Plasma on Wayland).

- Detect the current OS and available executables before issuing install,
  package-management, GUI, path, or shell commands.
- Use repository-relative paths in tracked files; use placeholders or environment
  variables instead of usernames or home directories.
- Keep runtime config machine-local. Install dependencies and build native or
  generated artifacts independently per OS — never share `node_modules`, virtual
  environments, or compiled output across machines.
- Preserve UTF-8 and executable bits; use `#!/usr/bin/env bash` for Bash scripts
  and keep Windows/Linux variants together via explicit `.ps1` / `.sh` entry points.
- Use path names valid on both systems (no Windows reserved names, trailing dots
  or spaces, invalid characters, or paths differing only by case).
