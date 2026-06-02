# lanp2p — LAN P2P file sharing (MVP)

Minimal C++17 tool for **UDP broadcast discovery** and **TCP file transfer** on a local network. Built with [Boost.Asio](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html).

**Architecture**：[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)（Module划分、依赖图、扩展方式）

## Modules

| Module | Description |
|--------|-------------|
| `lanp2p_common` | Domain types, configuration, default constants |
| `lanp2p_protocol` | LP2P line protocol encoding/decoding |
| `lanp2p_peer` | Peer registry |
| `lanp2p_index` | Shared directory index |
| `lanp2p_discovery` | UDP discovery |
| `lanp2p_transfer` | TCP server/client |
| `lanp2p_core` | Node orchestration (`Node`) |
| `lanp2p` (app) | CLI |

## Build

```bash
# Recommended: just will generate compile_commands.json (for clangd)
just build

# Or manually
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/apps/lanp2p/lanp2p --help
```

Dependencies: C++17, Boost ≥ 1.70, CMake ≥ 3.16.

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

Default ports: UDP `55555`, TCP `55556`.

## Roadmap (not in MVP)

- Block checksums and resume
- Multi-peer parallel blocks
- `sendfile()` zero-copy on Linux
- Unit tests + `install()` targets

## License

Personal experiment under `misc/idea/` — use at your own risk on trusted LANs only.
