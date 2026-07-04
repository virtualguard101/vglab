# Troubleshooting

## 在 vglab 根目录打开工作区时，子目录 C++ 项目 LSP / lint 报错

### 现象

- 工作区根目录是 **`vglab/`**。
- 实际代码在子路径里，例如 `misc/idea/p2p/`、`network/netstack/` 等。
- 编辑器里出现大量误报：找不到头文件（`boost/...`、`项目自己的 include/...`）、类型未定义、与真实 `cmake --build` 结果不一致。

### 原因

C/C++ 语言服务（**clangd** 或 Microsoft **C/C++** 扩展）需要知道**每个源文件对应的编译命令**（`-I`、`-std`、宏定义等）。

- 在**项目子目录**里打开文件夹时，clangd 会向上查找 `compile_commands.json`，往往能找到 `子项目/build/`。
- 在 **vglab 根**打开时，从 `misc/idea/p2p/libs/.../foo.cpp` 向上找，若中间没有编译数据库，就会用「空命令」做分析，于是全线飘红。

结论：**不是代码错了，是 LSP 没连上该子项目的编译数据库。**

---

### 为什么只配了 p2p，netstack 等子项目也能诊断？

**并不是因为只给 p2p 开了「特权」。** clangd 对**所有**子项目用的是同一套默认规则；差别在于子项目本地是否已有编译数据库，以及你是否在根目录 `.clangd` 里写了显式路由。

#### clangd 默认如何找 `compile_commands.json`

打开 netstack 的源文件时，clangd 会：

1. 读取 `vglab/.clangd`（若存在）；其中 **只有** `misc/idea/p2p/.*` 被显式指向 `misc/idea/p2p/build`，**不包含** netstack。
2. 若仍没有合适的数据库，则从该文件所在目录**向上**搜索祖先目录，并尝试：
   - `compile_commands.json`
   - 常见构建目录下的 `build/compile_commands.json`（现代 clangd 会扫 `build/` 子目录）

本机实测（clangd 22）对 netstack：

```text
Loaded compilation database from .../network/netstack/build/compile_commands.json
```

也就是说：**只要你在 netstack 里跑过 `just bootstrap` / `cmake -B build`，且缓存里打开了 `CMAKE_EXPORT_COMPILE_COMMANDS`，`build/compile_commands.json` 就会存在，clangd 在 vglab 根工作区下也能自动找到它**，不必在 `vglab/.clangd` 里再写一段 `PathMatch`。

#### 那 p2p 为什么还要单独配置？

| 情况 | 说明 |
|------|------|
| **当时还没有 `build/compile_commands.json`** | 未 `cmake` 配置/构建时，clangd 无命令可用，Boost、`lanp2p/...` 头文件会全线报错。 |
| **依赖更重** | p2p 依赖 Boost.Asio 等，比 netstack（主要是 `-Iinclude` + C++20）更容易在「空配置」下暴露误报。 |
| **显式 `vglab/.clangd` 是兜底** | `PathMatch: misc/idea/p2p/.*` 明确指定数据库路径，不依赖「是否记得扫到 `build/`」；与 `p2p/compile_commands.json` 符号链接、`just bootstrap` 一起，保证可复现。 |
| **并非替代默认发现** | p2p 即使删掉根 `.clangd` 里那一段，在已有 `misc/idea/p2p/build/compile_commands.json` 时，clangd 往往也能像 netstack 一样自动找到 `build/`。 |

#### 各子项目应对照检查

```bash
# 在子项目根执行，确认数据库存在
test -f build/compile_commands.json && echo OK || echo MISSING
```

| 子项目 | 典型路径 | 根 `.clangd` 显式路由 | 通常能否自动发现 `build/` |
|--------|----------|------------------------|---------------------------|
| lanp2p | `misc/idea/p2p` | 有 | 能（已有 DB 时） |
| netstack | `network/netstack` | 无 | 能（已 `just bootstrap` 时） |

**建议（新建子项目）：**

1. 在子项目 `CMakeLists.txt` 写 `set(CMAKE_EXPORT_COMPILE_COMMANDS ON)`（不要只靠 CMakeCache 里曾经的 `-D`）。
2. `just bootstrap` 生成 `build/compile_commands.json`。
3. 若结构标准（`<proj>/build`），**可以不**改 `vglab/.clangd`；若构建目录非 `build`、或多个 target 目录，再在根 `.clangd` 加 `PathMatch`。

---

### 通用解决流程（适用于新建 CMake C++ 子项目）

设子项目根目录为 `<proj>`（相对 vglab 根），例如 `misc/idea/p2p`；构建目录默认为 `<proj>/build`。

#### 1. 让 CMake 导出 `compile_commands.json`

在子项目顶层 `CMakeLists.txt` 中：

```cmake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```

配置并构建一次：

```bash
cd <proj>
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build -j
```

确认存在：`<proj>/build/compile_commands.json`。

可选：在子项目根做符号链接，便于工具查找：

```bash
ln -sfn build/compile_commands.json <proj>/compile_commands.json
```

（建议把 `compile_commands.json` 加入子项目 `.gitignore`。）

#### 2. 在 vglab 根添加 `.clangd` 路由（推荐）

编辑 **`vglab/.clangd`**，为每个子项目增加一段（可多段并列）：

```yaml
---
If:
  PathMatch: <proj>/.*          # 例: misc/idea/p2p/.*
CompileFlags:
  CompilationDatabase: <proj>/build
```

- `PathMatch`：相对 **vglab 仓库根** 的路径前缀。
- `CompilationDatabase`：含 `compile_commands.json` 的目录（通常是 `build/`）。

clangd 会从被打开的文件路径向上合并配置；匹配到 `PathMatch` 的源文件会使用对应数据库。

#### 3. 在子项目内添加 `.clangd`（可选）

在 **`<proj>/.clangd`** 中写入（单独只打开子文件夹时也有效）：

```yaml
CompileFlags:
  CompilationDatabase: build
```

#### 4. 工作区 VS Code / Cursor 设置

编辑 **`vglab/.vscode/settings.json`**：

```json
{
  "C_Cpp.intelliSenseEngine": "disabled",
  "C_Cpp.default.compileCommands": "${workspaceFolder}/<proj>/build/compile_commands.json"
}
```

说明：

- 使用 **clangd** 时建议关闭 MS C++ IntelliSense，避免两套诊断重复、矛盾。
- `default.compileCommands` 只能指向**一个** JSON；多子项目时更可靠的是靠根目录 **`.clangd` 的 `PathMatch`** 分流。
- 推荐扩展见 `vglab/.vscode/extensions.json`（如 `llvm-vs-code-extensions.vscode-clangd`）。

#### 5. 刷新 LSP

```bash
cd <proj> && cmake -S . -B build && cmake --build build -j
```

在编辑器命令面板执行：**`clangd: Restart language server`**（或重载窗口）。

验证（可选）：

```bash
clangd --check=<proj>/path/to/any.cpp \
  --compile-commands-dir=<proj>/build
```

输出中应出现 `Loaded compilation database from .../build/compile_commands.json`，且无大量 parse error。

---

### 非 CMake 项目（简要）

| 构建方式 | 做法 |
|----------|------|
| **Bear / compiledb** | `bear -- make` 生成 `compile_commands.json`，再按上文放到 `<proj>/build` 或根并配置 `.clangd` |
| **Meson** | `meson setup build` 会在 build 目录生成 `compile_commands.json` |
| **仅头文件 + 手写** | 在 `<proj>/.clangd` 用 `CompileFlags.Add` 补 `-I`、`-std`（维护成本高，不推荐大项目） |

---

### 常见问题

| 问题 | 处理 |
|------|------|
| 改了 `CMakeLists.txt` 仍报旧错 | 重新 `cmake` 配置并 **Restart clangd** |
| 只有部分文件报错 | 该文件可能不在 `compile_commands.json` 里（未加入 target）；检查 CMake `add_library` / `add_executable` |
| 与 MS C++ 扩展双重波浪线 | 设置 `"C_Cpp.intelliSenseEngine": "disabled"`，只保留 clangd |
| `compile_commands.json` 里没有第三方 `-I` | 确认 CMake `target_link_libraries(... PUBLIC deps)`；若仍缺失但本机能编译，可能是系统头路径（如 `/usr/include`）已被编译器默认搜索 |
| 新增第二个 C++ 子项目 | 先保证 `<proj>/build/compile_commands.json`；仅当自动发现不够时，再在 `vglab/.clangd` **追加** `PathMatch` |
| netstack 能诊断、p2p 不能 | 对比两者是否都执行过 `cmake -B build` 且存在 `build/compile_commands.json` |

---

### 仓库内实例

#### lanp2p（显式路由 + 标准 `build/`）

| 项 | 路径 |
|----|------|
| 子项目 | `misc/idea/p2p` |
| 编译数据库 | `misc/idea/p2p/build/compile_commands.json` |
| 生成命令 | `cd misc/idea/p2p && just bootstrap` |
| 根路由（可选兜底） | `vglab/.clangd` → `PathMatch: misc/idea/p2p/.*` |
| 说明 | [misc/idea/p2p/README.md](misc/idea/p2p/README.md) |

#### netstack（仅靠自动发现即可）

| 项 | 路径 |
|----|------|
| 子项目 | `network/netstack` |
| 编译数据库 | `network/netstack/build/compile_commands.json` |
| 生成命令 | `cd network/netstack && just bootstrap` |
| 根路由 | 无（clangd 自动加载 `build/compile_commands.json`） |
| CMake 建议 | 在 `CMakeLists.txt` 显式 `set(CMAKE_EXPORT_COMPILE_COMMANDS ON)`，避免换机器后 Cache 未导出 |

新建子项目时：**优先**保证 `build/compile_commands.json`；只有自动发现不够时，再仿 lanp2p 在 `vglab/.clangd` 增加 `PathMatch`。
