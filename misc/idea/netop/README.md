# netop

**netop** 是 htop / nvtop 风格的 **Linux 网络资源监控 + 轻量控制** TUI。MVP 聚焦两件事：

1. **接口视图** — 从 `/proc/net/dev` 实时计算 RX/TX 带宽、PPS、错误与丢包  
2. **进程视图** — 将 `/proc/net/tcp|udp` 与 `/proc/<pid>/fd` 关联，按 socket 活动度排序，并支持 SIGTERM 终止进程  

> 平台：Linux（依赖 procfs）。macOS / Windows 为后续扩展。

## 快速开始

```bash
cd misc/idea/netop
just run          # 或 cargo run
just run -- -i 1000 -p   # 1s 刷新，启动即进程视图
just release      # 优化构建 → target/release/netop
```

## 快捷键

| 键 | 作用 |
|----|------|
| `1` / `2` | 接口 / 进程视图 |
| `Tab` | 切换视图 |
| `j` / `k` 或 `↑` / `↓` | 移动选择 |
| `s` | 循环切换排序键 |
| `/` | 过滤（Enter 确认，Esc 取消） |
| `Space` | 暂停 / 恢复采样 |
| `F9` | 对选中进程发送 SIGTERM（需确认） |
| `?` | 显示帮助 |
| `q` / `Ctrl+C` | 退出 |

## 权限说明

| 功能 | 所需权限 |
|------|----------|
| 接口带宽、PPS | 普通用户 |
| 进程 socket 枚举 | 普通用户（仅能看见有权限的 `/proc/<pid>`） |
| 终止进程 | 对目标进程的所有者或 root |

进程 **Activity/s** 列在 MVP 中为 socket 队列字节变化的代理指标，并非 eBPF 级精确 per-process 带宽；精确流量归因见 [架构文档](docs/ARCHITECTURE.md) 的进阶路线。

## 项目结构

```
netop/
├── Cargo.toml
├── README.md
├── justfile
├── docs/ARCHITECTURE.md
└── src/
    ├── main.rs
    ├── cli.rs
    ├── util.rs
    ├── app/           # TUI 主循环（MVU）
    ├── collector/     # 数据采集
    └── control/       # kill 等控制动作
```

## 相关竞品

`nethogs`、`iftop`、`bmon`、`nvtop`（GPU）、GitHub 上基于 BPF 的 netop 变体。

**差异化方向**：监控 + 控制一体化、多视图 TUI、后续 eBPF / tc 集成。

## License

MIT
