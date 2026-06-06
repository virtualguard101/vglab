# Lab Guidelines

As [README.md](README.md), this repo is a lab repo for Computer Science self-learning. Contains codes of common scripts, teaching code, minimal verification, PoCs(Proof of Concepts), and MVPs(Minimum Viable Products) for Ideas. Mainly writen in C++, Python, Go, and Rust.

This repo follows the following guidelines:

- [README.md](README.md) — How to clone and *bootstrap* the environment

- [TROUBLESHOOTING.md](TROUBLESHOOTING.md) — C++ LSP / clangd and other *troubleshootings*.

---

## Design Principles

| Principle | Description |
|-----------|-------------|
| **Project Autonomy** | Each runnable unit has its own build and `justfile` (or language default commands), and the root directory does not compile all subprojects uniformly |
| **Convention over Configuration** | Reuse the same template for similar experiments; only add root directory `.clangd` etc. when automatic discovery is not enough |
| **Maturity Leveling** | Use labels to distinguish sketch / poc / mvp / teaching / legacy, avoid using the same standard for all directories |
| **Just Enough Documentation** | Small demos only need a short README; for projects that grow, add `docs/`, `ARCHITECTURE.md`, and ADR |

**Intentionally not done**: Root level Bazel / single mega-build CMake, forced uniform test framework, writing architecture documentation for every 50-line demo, and full-repo clang-tidy zero warnings.

---

## Top-level Directories

| Path | Purpose |
|------|------|
| `ai/` | AI-related |
| `cca/` | Computer Architecture |
| `database/` | Database |
| `dsa/` | Data Structure and Algorithm |
| `infrastructure/` | Infrastructure / Cloud Native |
| `lang/` | PL(Programming Language), Syntax, Small Demos (can be thrown away, can be bad) |
| `misc/idea/` | Ideas MVPs / PoCs |
| `misc/legacy/` | Archived, Some Miscellaneous Code |
| `network/` | Computer Network, Protocol PoCs, etc. |
| `os/` | Operating System |
| `scripts/` | Cross-project Scripts |

`drafts/` and `trash/` are only local drafts, and are ignored in `.gitignore`.

---

## Subproject Contract

Each subproject that **hopes to be runnable** should:

| Element | Description |
|------|------|
| `README.md` | Short description: what it is, dependencies, how to build / test / run |
| One-click command | Prefer `just bootstrap`, `just build`, `just test`; or the language-specific command |
| Build artifacts not stored | `build/`, `target/`, etc.; root `.gitignore` already covers common items |
| Maturity label | Indicate in the README first, see the table below |

---

## Mainly PL Standards Benchmark

Should copy the same template for similar experiments, but **not necessarily the same version number**.

### C++

- `CMakeLists.txt`，set `CMAKE_EXPORT_COMPILE_COMMANDS ON`:

  ```cmake
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  ```

- Recommended `justfile`:

  ```just
  bootstrap:
      cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
      ln -sfn build/compile_commands.json compile_commands.json

  build: bootstrap
      cmake --build build -j

  test:
      ctest --test-dir build
  ```

- When the project is large, refer to `libs/` + `apps/` + `docs/ARCHITECTURE.md` (e.g. `misc/idea/p2p`, `network/netstack`).

- **LSP**: After generating `build/compile_commands.json`, clangd in the vglab root workspace can usually automatically discover `build/`; see [TROUBLESHOOTING.md](TROUBLESHOOTING.md#在-vglab-根目录打开工作区时子目录-C-项目-LSP--lint-报错). Only when the build directory is not `build/` or automatic discovery fails, add `PathMatch` to the root `.clangd`.

Self-check:

```bash
test -f build/compile_commands.json && echo OK || echo MISSING
```

### Rust

- **Teaching Fragments**: Consider using **workspace** to aggregate multiple small crates in `lang/rust/Cargo.toml`, for easier `cargo test -p <name>`.

- **Idea / Independent Tools**: Use independent `Cargo.toml` in `misc/idea/<name>/`.

- `Cargo.lock`: Teaching demos can be omitted; for MVPs that are intended to be maintained long-term, it is recommended to submit.

### Go

- Each demo has an independent `go.mod` (current convention).

- Optional: Use `use ./hello ./echo ...` in `lang/golang/go.work` to make it easier to use gopls / `go work sync` in the repository root.

### Python

- The root [pyproject.toml](pyproject.toml) + `uv sync` provides a common environment for notebooks, etc.; for **heavy dependencies** (e.g. torch), it is recommended to:

  - Use optional dependency groups, or

  - Put heavy dependencies in a separate `pyproject.toml` in the subproject (e.g. `ai/agent/tiny-agent`).

- Avoid forcing all clonees of this repo to install the full GPU / deep learning stack.

---

## Root `justfile`

The root [justfile](justfile) is responsible for **repository-level** operations (e.g. git aliases), and does not replace the `justfile` in the subproject.

Can gradually increase delegated commands, for example:

```just
# List subdirectories with justfile (need to add recipe in the root justfile)
list-projects:
    @find . -name justfile -not -path './.git/*' | sed 's|/justfile||' | sort

netstack:
    just -f network/netstack/justfile build

p2p:
    just -f misc/idea/p2p/justfile build
```

After the number of projects increases, consider `projects.yaml` + `scripts/vglab list|test <name>` for discovery; **do not implement it initially**.

---

## IDE / LSP

| Language | Suggestion |
|------|------|
| C++ | `compile_commands.json` + [TROUBLESHOOTING.md](TROUBLESHOOTING.md); for new subprojects, default to not writing root `.clangd`, and run `just bootstrap` first |
| Go | `go.work` (optional) |
| Rust | `rust-analyzer` for workspace or single crate is default |
| Python | Subproject README should mention `uv run` / venv; for necessary, subdirectory `pyrightconfig.json` |

**Note**: When opening the workspace in the vglab root, if the subproject has `<proj>/build/compile_commands.json` (e.g. `network/netstack`), clangd can usually **automatically** load, and it is not required to configure each project separately in the root `.clangd`. Explicit `PathMatch` (e.g. `misc/idea/p2p`) is used for fallback and reproducibility.

---

## CI (optional)

Only enable path-filter for projects that are still being maintained, to avoid building the entire repo at once:

- `network/netstack/**` changed → `just -f network/netstack/justfile test`

- `misc/idea/p2p/**` changed → build / test for p2p

- `lang/rust/**` changed → `cargo test` (workspace or specific crate)

Do not use a single CMake in the root directory to build the entire vglab.

---

## Repository Hygiene

| Problem | Suggestion |
|------|------|
| Course/third-party vendored code (e.g. `cs168/.../simulator/lib/`) | Submodule, separate branch, or clearly marked "do not change"; reduce IDE indexing interference |
| `misc/legacy/` | README should mention the archive date, and the build is not guaranteed |
| AI-assisted MVP | Put in `misc/idea/`, keep the minimum executable path and README |
| Root `uv.lock` too heavy | Split optional dependencies or down to subprojects |

---

## Repository Reference Examples

| Project | Path | Language | Description |
|------|------|------|------|
| lanp2p | `misc/idea/p2p` | C++ | Modular libs + apps; root `.clangd` has explicit routing |
| netstack | `network/netstack` | C++ | Layered network stack MVP; rely on `build/compile_commands.json` for automatic LSP |
| operator-rs | `misc/idea/operator-rs` | Rust | Independent crate |
| tiny-agent | `ai/agent/tiny-agent` | Python | Independent `pyproject.toml` |

When creating a new C++ subproject, you can copy the CMake + `just bootstrap` mode of netstack / p2p; only check [TROUBLESHOOTING.md](TROUBLESHOOTING.md) and consider the root `.clangd` when the LSP is still abnormal.

---

## Suggested Evolution Order

- **Mid-term**: `lang/rust` workspace, `lang/golang/go.work`; Python dependency splitting; CI path-filter for 2~3 active projects.

- **Long-term** (when there are many projects): `projects.yaml` + `scripts/vglab`; expand the root `just list-projects` as needed.

---

