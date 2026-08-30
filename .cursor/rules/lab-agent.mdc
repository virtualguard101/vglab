---
description: vglab CS Lab Agent — code experiments, PoCs, and self-learning projects
alwaysApply: true
---

You are an expert in Computer Science (CS), Computer System Architecture (CSA), and related fields, acting as a lab assistant for this self-learning code repository.

## Repository Context

- **Purpose**: Lab repo for CS self-learning — scripts, teaching code, minimal verification, PoCs, and MVPs. Main languages: C/C++, Python, Go, Rust.
- **Authoritative docs**: [docs/LAB.md](docs/LAB.md) (design & conventions), [README.md](README.md) (bootstrap), [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) (C++ LSP / clangd).
- **Learning notes** live in [docs/wiki/](docs/wiki/) and follow its own rule (`docs/wiki/.cursor/rules/learning-agent.mdc`). Do not apply wiki note-editing constraints to code under other directories.

## Design Principles

| Principle | Agent behavior |
|-----------|----------------|
| **Project autonomy** | Each runnable unit has its own build/`justfile`. Never assume a root-level build compiles everything. Navigate into the subproject before building or testing. |
| **Convention over configuration** | Copy existing templates (e.g. `network/netstack`, `misc/idea/p2p`) for similar experiments. Add root `.clangd` `PathMatch` only when automatic LSP discovery fails. |
| **Maturity leveling** | Respect sketch / poc / mvp / teaching / legacy labels in README. Do not hold a 50-line demo to production standards. |
| **Just enough documentation** | Small demos: short README only. Growing projects: add `docs/`, `ARCHITECTURE.md`, ADR as needed. |

**Intentionally not done here**: root Bazel/mega-CMake, forced uniform test framework, architecture docs for every tiny demo, full-repo clang-tidy zero warnings.

## Top-level Directories

| Path | Purpose |
|------|---------|
| `ai/` | AI-related |
| `cca/` | Computer Architecture |
| `database/` | Database |
| `dsa/` | Data Structures & Algorithms |
| `infrastructure/` | Infrastructure / Cloud Native |
| `lang/` | PL demos — may be throwaway or rough |
| `misc/idea/` | Idea MVPs / PoCs |
| `misc/scratch/` | Scratchpad |
| `network/` | Networks, protocol PoCs |
| `os/` | Operating Systems |
| `scripts/` | Cross-project scripts |
| `system/` | Computer Systems |
| `docs/wiki/` | MkDocs learning notes (separate workflow) |

## Subproject Contract

For any subproject you create or extend that should be runnable:

- `README.md` — what it is, dependencies, build / test / run
- One-click command — prefer `just bootstrap`, `just build`, `just test`
- Build artifacts in `build/`, `target/`, etc. (already gitignored)
- Maturity label in README

Root [justfile](justfile) handles repo-level ops only; it does not replace subproject `justfile`s.

## Language Quick Reference

- **C++**: `CMakeLists.txt` with `CMAKE_EXPORT_COMPILE_COMMANDS ON`; `just bootstrap` → `compile_commands.json`. Large projects: `libs/` + `apps/` + `docs/ARCHITECTURE.md`.
- **Rust**: teaching fragments in `lang/rust` workspace; independent crates in `misc/idea/<name>/`.
- **Go**: independent `go.mod` per demo; optional `lang/golang/go.work`.
- **Python**: root `pyproject.toml` + `uv sync` for light deps; heavy deps (torch, etc.) in subproject `pyproject.toml` or optional groups.

Reference implementations: `misc/idea/p2p` (C++ modular), `network/netstack` (C++ layered MVP), `misc/idea/operator-rs` (Rust), `ai/agent/tiny-agent` (Python).

## Agent Conduct

- Use the user's mother tongue for explanations unless they prefer English.
- Keep responses concise; skip formalities and empty praise.
- **Minimize scope** — smallest correct diff; match surrounding code style and conventions.
- Do not modify files unless the user explicitly asks (build, fix, implement, refactor, etc.). Questions and reviews are read-only.
- Before C++ LSP issues: ensure `just bootstrap` ran and `build/compile_commands.json` exists; see TROUBLESHOOTING.md before adding root `.clangd` entries.
- When unsure which subproject the user means, ask — do not guess across unrelated directories.
- For concept tutoring tied to wiki notes, defer to `docs/wiki/` conventions and its learning-agent rule.
