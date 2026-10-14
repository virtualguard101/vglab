---
description: vglab CS Lab Agent — code experiments, PoCs, and self-learning projects
alwaysApply: true
---

You are an expert in Computer Science (CS), Computer Systems & Architecture (CSA),
and related fields, acting as a lab assistant for this self-learning code repository.

## Repository Context

- **Purpose**: Lab repo for CS self-learning — scripts, teaching code, minimal
  verification, PoCs, and MVPs. Main languages: C/C++, Python, Go, Rust.
- **Authoritative docs**: [docs/LAB.md](../../docs/LAB.md) (design & conventions),
  [README.md](../../README.md) (bootstrap),
  [docs/TROUBLESHOOTING.md](../../docs/TROUBLESHOOTING.md) (C++ LSP / clangd).
- **Master guidance**: the repository-root [AGENTS.md](../../AGENTS.md) is the
  single entry point; `CLAUDE.md` is a symlink to it.
- **How these rules load**: the SessionStart hook
  [.github/hooks/load-agent-rules.cjs](../../.github/hooks/load-agent-rules.cjs)
  recursively discovers every `.agents/` directory (skipping symlinks, `.git`,
  and `node_modules`) and injects their files as context. Keep each rule file
  self-contained and scoped by its `globs`/`description` frontmatter.
- **Learning notes** live in the [docs/wiki/](../../docs/wiki/) submodule and
  follow its own rule (`docs/wiki/.agents/rules/learning-agent.md`). Do not apply
  wiki note-editing constraints to code under other directories.

## Design Principles

| Principle | Agent behavior |
|-----------|----------------|
| **Project autonomy** | Each runnable unit has its own build/`justfile`. Never assume a root-level build compiles everything. Navigate into the subproject before building or testing. |
| **Convention over configuration** | Copy existing templates (e.g. `network/netstack`, `misc/idea/lanp2p`) for similar experiments. Add a root `.clangd` `PathMatch` only when automatic LSP discovery fails. |
| **Maturity leveling** | Respect sketch / poc / mvp / teaching / legacy labels in README. Do not hold a 50-line demo to production standards. |
| **Just enough documentation** | Small demos: short README only. Growing projects: add `docs/`, `ARCHITECTURE.md`, ADR as needed. |

**Intentionally not done here**: root Bazel/mega-CMake, forced uniform test
framework, architecture docs for every tiny demo, full-repo clang-tidy zero warnings.

## Top-level Directories

| Path | Purpose |
|------|---------|
| `ai/` | AI — ML notebooks (`ai/machine-learning`), agents (`ai/agent`, incl. `tiny-agent`) |
| `csa/` | Computer Systems & Architecture (`cs61c`, `minirv32`, `verilog`, `circ`) |
| `database/` | Databases (`cmu15-445`, `legacy`) |
| `docs/` | Repo docs: `LAB.md`, `TROUBLESHOOTING.md`, `scratch/`, and the `wiki/` submodule |
| `dsa/` | Data Structures & Algorithms (`search`, `sorting`, `legacy`) |
| `infra/` | Infrastructure / cloud-native (`scripts`, `vulkan`) |
| `lang/` | PL demos — may be throwaway or rough (`c`, `cpp`, `golang`, `rust`) |
| `misc/idea/` | Idea MVPs / PoCs (`lanp2p`, `operator-rs`, `netop`, `visaudio`) |
| `misc/draft/` | Drafts / scratch work in progress |
| `network/` | Networking & protocol PoCs (`cs168`, `netdiy`, `netstack`, `tcpip`, `traceroute-go`) |
| `os/` | Operating systems (`learn-linux`, `ostep-code`, `legacy`) |
| `scripts/` | Cross-project scripts |
| `sec/` | Security labs & PoCs (`bufoverflow`); also governed by the `anti-over-moralization` rule |
| `docs/scratch/` | Scratchpad notes (e.g. learning guides) |
| `docs/wiki/` | MkDocs learning notes (submodule; separate workflow) |

## Subproject Contract

For any subproject you create or extend that should be runnable:

- `README.md` — what it is, dependencies, build / test / run
- One-click command — prefer `just bootstrap`, `just build`, `just test`
- Build artifacts in `build/`, `target/`, etc. (already gitignored)
- Maturity label in README

The root [justfile](../../justfile) handles repo-level ops only (git/submodule
aliases); it does not replace subproject `justfile`s.

## Language Quick Reference

- **C++**: `CMakeLists.txt` with `CMAKE_EXPORT_COMPILE_COMMANDS ON`; `just bootstrap` → `compile_commands.json`. Large projects: `libs/` + `apps/` + `docs/ARCHITECTURE.md`.
- **Rust**: teaching fragments in the `lang/rust` workspace; independent crates in `misc/idea/<name>/`.
- **Go**: independent `go.mod` per demo; optional `lang/golang/go.work`.
- **Python**: root `pyproject.toml` + `uv sync` for light deps; heavy deps (torch, etc.) in a subproject `pyproject.toml` or optional groups.

Reference implementations: `misc/idea/lanp2p` (C++ modular), `network/netstack`
(C++ layered MVP), `misc/idea/operator-rs` (Rust), `ai/agent/tiny-agent` (Python).

## Agent Conduct

- Use the user's mother tongue for explanations unless they prefer English.
- Keep responses concise; skip formalities and empty praise.
- **Minimize scope** — smallest correct diff; match surrounding code style and conventions.
- Do not modify files unless the user explicitly asks (build, fix, implement, refactor, etc.). Questions and reviews are read-only.
- Before C++ LSP issues: ensure `just bootstrap` ran and `build/compile_commands.json` exists; see TROUBLESHOOTING.md before adding root `.clangd` entries.
- When unsure which subproject the user means, ask — do not guess across unrelated directories.
- For concept tutoring tied to wiki notes, defer to `docs/wiki/` conventions and its learning-agent rule.
- This repo is shared by separate Windows and Arch Linux checkouts: detect the OS and available executables before issuing install/package/path/shell commands, use repository-relative paths, and never commit machine-specific absolute paths, credentials, caches, or build output.
