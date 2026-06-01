# lanp2p — LAN P2P file sharing (MVP)

Minimal C++17 tool for **UDP broadcast discovery** and **TCP file transfer** on a local network. Built with [Boost.Asio](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html).

**架构文档**：[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)（模块划分、依赖图、扩展方式）

## 模块概览

| 库 | 职责 |
|----|------|
| `lanp2p_common` | 领域类型、配置、默认常量 |
| `lanp2p_protocol` | LP2P 线协议编解码 |
| `lanp2p_peer` | Peer 注册表 |
| `lanp2p_index` | 共享目录索引 |
| `lanp2p_discovery` | UDP 发现 |
| `lanp2p_transfer` | TCP 服务端 / 客户端 |
| `lanp2p_core` | 节点编排 (`Node`) |
| `lanp2p` (app) | CLI |

## Build

```bash
# 推荐：just 会生成 compile_commands.json（供 clangd 使用）
just build

# 或手动
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/apps/lanp2p/lanp2p --help
```

依赖：C++17、Boost ≥ 1.70、CMake ≥ 3.16。

## CLI

| Command | Description |
|---------|-------------|
| `run` | Serve shared directory, discover peers, accept downloads |
| `scan` / `peers` | Discover peers (UDP only, ephemeral port) |
| `local-list` | List files in local share directory |
| `list --host H` | List remote peer's shared files |
| `download --host H --remote PATH` | Download a file |

## Quick test (two terminals)

```bash
mkdir -p share downloads
echo "hello lan" > share/test.txt
./build/apps/lanp2p/lanp2p run --share-dir ./share

# other terminal
./build/apps/lanp2p/lanp2p scan
./build/apps/lanp2p/lanp2p list --host <peer-ip>
./build/apps/lanp2p/lanp2p download --host <peer-ip> --remote test.txt
```

Default ports: UDP `47777`, TCP `47778`.

## Roadmap (not in MVP)

- Block checksums and resume
- Multi-peer parallel blocks
- `sendfile()` zero-copy on Linux
- Unit tests + `install()` targets

## License

Personal experiment under `misc/idea/` — use at your own risk on trusted LANs only.
