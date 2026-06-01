# Netstack 教学重写 — 计划与建议

本文档记录以 C++ / Rust 重写 Netstack（教学/学习用途）的范围、里程碑与工程实践。  
架构对照见 [`references/architecture.md`](references/architecture.md)。

---

## 共通原则（两种语言均适用）

### 学习目标

- 掌握 **分层 + 可插拔协议注册**（对标 `tcpip/stack`）
- 理解 **非阻塞 Endpoint + 等待队列** 或明确的 async 模型
- 能跑通 **channel link 测试** 与可选 **TUN demo**

### 范围边界（第一期不做）

- [ ] 完整 iptables / NAT / 与 Linux 全量 `setsockopt` 对齐
- [ ] 多 NIC 热插拔、完整 IPv6 / NDP（除非单列扩展 milestone）
- [ ] 生产级拥塞控制（CUBIC/BBR 等，TCP 阶段先固定窗口或 NewReno 简化版）
- [ ] 与 gVisor 主线功能 parity

### 推荐里程碑

| 阶段 | 范围 | 验收 |
|------|------|------|
| M0 | 链路环回 + IPv4 解析/校验 | 单元测试喂原始帧（详见 [`docs/m0.md`](docs/m0.md)） |
| M1 | UDP echo | `channel` 式 fake link，无 TUN（详见 [`docs/m1.md`](docs/m1.md)） |
| M2 | TCP 单连接（简化版） | 与参考用例或 `nc` 少量场景对比 |
| M3 | TUN 对接宿主 | `tun_tcp_echo` 级别 demo |
| 扩展 | ARP、分片、IPv6、iptables | 单独 milestone，不阻塞主线 |

### 推荐目录结构（逻辑分层）

```text
netstack
├── docs
│   └── adr              # 关键设计决策（缓冲、并发、TCP 简化假设）
├── <lang>
│   ├── netstack         # 根类型、Error、Address（对标 tcpip）
│   ├── stack            # Stack, NIC, Route, 分发
│   ├── header           # 纯解析/构造/校验和
│   ├── buffer
│   ├── link             # loopback, channel, tun
│   ├── net              # ipv4, arp, fragmentation
│   ├── transport        # udp, tcp
│   ├── ports
│   └── seqnum
├── adapters             # 可选：类似 gonet 的 API 封装
├── examples
└── tests
    └── integration
```

### 架构要点（继承 Netstack）

- [ ] **header 无状态**：解析/校验和可单测 + fuzz，不持全局状态
- [ ] **协议可注册**：避免到处 `switch(proto)`；Rust trait / C++ 小接口 + 注册表
- [ ] **测试优先 channel link**：不依赖 root，CI 稳定
- [ ] **缓冲区策略文档化**：先清晰（链式 buffer / 可 prepend），再优化零拷贝
- [ ] **阻塞模型二选一**：非阻塞 + wait queue，或内部非阻塞 + 对外 async；写 ADR，不混用
- [ ] **统一错误与统计**：对标 `tcpip.Error` + 分层 `Stats`

### 测试与质量（共通 checklist）

- [ ] 单元测试：header、checksum、seqnum 回绕、路由表
- [ ] 集成测试：channel link 上 UDP/TCP 握手
- [ ] 可选：固定 pcap / golden frame 回归
- [ ] Fuzz：IPv4/TCP/UDP 头解析
- [ ] 特性开关：`tcp` / `tun` 等未成熟模块默认关闭
- [ ] 每 milestone 更新 `architecture.md` 的「已实现 / 未实现」表

### 教学实验顺序

1. [ ] 以太网/IP 头库 + RFC 测试向量
2. [ ] Stack + loopback NIC，IPv4 本地交付
3. [ ] UDP：端口表 + echo
4. [ ] TCP 极简：SYN → data → FIN（固定窗口，无 SACK）
5. [ ] RTO、滑动窗口、一种拥塞控制写透
6. [ ] TUN demo（最后接宿主）

### 常见坑

- [ ] 避免第一期完整 TCP；拆成解析 / 状态机 / 定时器 / 重传
- [ ] 不追求与 Linux 行为逐条对齐；测试用例自建
- [ ] 避免整栈一把大锁；区分数据面与控制面
- [ ] 必须先有 channel link 再依赖 TUN
- [ ] seqnum 回绕单独模块 + 属性测试
- [ ] 文档与实现同步（ADR + architecture）

### 参考资源

| 资源 | 用途 |
|------|------|
| `references/` (Netstack) | 分层与接口主参考 |
| [smoltcp](https://github.com/smoltcp-rs/smoltcp) | Rust 模块粒度与实现深度参考 |
| [gVisor pkg/tcpip](https://github.com/google/gvisor) | 演进后的 API（体量较大） |
| lwIP (C) | 极简栈思路（API 较老，仅概念参考） |

**总原则**：接口设计跟 Netstack，实现深度按 smoltcp 裁剪，测试用 channel link 驱动。

---

## C++ 重写建议

### 语言与标准

- [ ] **C++17 起步**（`std::span`、`std::optional`、`std::variant`）；必要时 C++20
- [ ] **禁止**未文档化的裸 `new/delete`；资源用 RAII + `std::unique_ptr`
- [ ] 跨层传递缓冲区用 `std::span<const uint8_t>` 或自有 `BufferChain` + 明确所有权文档
- [ ] 阻塞模型：教学推荐 **非阻塞 Endpoint + 自研 WaitQueue**（更贴近 Netstack）；或 `std::future` 简化版

### 工程实践

- [ ] CMake 按 target 拆分：`header`、`stack`、`link`、`net`、`transport`…
- [ ] `-Wall -Wextra -Werror`；Debug 构建开 **ASan + UBSan**
- [ ] `clang-format` + `clang-tidy` 进 CI
- [ ] 接口用纯虚类 / concept（C++20）对标 Go interface；注册表 `unordered_map<ProtocolNumber, ...>`
- [ ] 头文件与实现分离；`header/` 尽量无全局状态

### 推荐 target 划分

```text
.
├── include
│   └── netstack
├── src
│   ├── header
│   ├── buffer
│   ├── stack
│   ├── link
│   ├── net
│   ├── transport
│   ├── ports
│   └── seqnum
├── examples
└── tests
```

### C++ 特有任务

- [ ] 写清 **所有权 ADR**（谁拥有 packet buffer、何时释放）
- [ ] 避免整个 `Stack` 单锁；数据面用 per-NIC 或 per-layer 细粒度锁（先文档后实现）
- [ ] `seqnum` 命名空间 + 回绕比较函数，单测对齐 Netstack `seqnum` 语义
- [ ] libFuzzer 目标：`header` 解析
- [ ] 示例：`examples/udp_echo`、`examples/tun_tcp_echo`（Linux + TUN）

### C++ 参考对照

- [ ] 与 `references` 对照表（`docs/adr/001-cpp-layout.md`）
- [ ] 字节布局与 Wireshark 对照实验（教学加分项）

---

## 决策记录（待填）

| 项 | Rust | C++ | 决定 |
|----|------|-----|------|
| 阻塞模型 | async / waiter | wait queue / future | |
| 缓冲区 | | | |
| 构建系统 | cargo workspace | CMake | |
| 首期传输层 | UDP only → TCP | 同左 | |
| TUN 引入时机 | M3 | M3 | |

---

## Rust 重写建议

### 语言与生态选型

- [ ] 主线：`std` + workspace 多 crate（见上方目录）
- [ ] 进阶（可选）：`no_std` + `heapless`/`embedded-io` 分支，与主线 API 对齐
- [ ] 阻塞模型：推荐 **内部非阻塞 + `async` 对外**（`tokio` 或轻量 executor）；若对标 Netstack，则实现 `Waiter` trait + `Waker`
- [ ] TUN：[`tun`](https://crates.io/crates/tun) 等成熟 crate，M3 再引入

### 工程实践

- [ ] `cargo fmt` / `clippy -D warnings` 进 CI
- [ ] 所有 `unsafe` 必须注释 SAFETY 与不变量；考虑 `miri` 测 unsafe 路径
- [ ] `cargo fuzz` 覆盖 `header` 解析
- [ ] 公开 API 仅 `stack` + `Endpoint`；其余 `pub(crate)`
- [ ] 用 trait 对标 Netstack 的 `LinkEndpoint` / `NetworkProtocol` / `TransportProtocol`

### 推荐 crate 划分

```text
crates
├── netstack             # Address, Error, Endpoint trait
├── stack
├── header
├── buffer
├── link
├── net
├── transport
├── ports
├── seqnum
└── adapters             # 可选：类似 std::net 的封装
```

### Rust 特有任务

- [ ] 定义 `PacketBuffer` 所有权模型（`Bytes` / 自研 chain / `Vec` 起步）
- [ ] `seqnum` 用 newtype + 单元测试覆盖回绕
- [ ] TCP 状态机用 `enum` + 显式转换表，避免散落 `bool`
- [ ] 集成测试：`tokio::test` + channel link
- [ ] 示例：`examples/udp_echo.rs`、`examples/tun_tcp_echo.rs`

### Rust 参考对照

- [ ] 阅读 smoltcp 的 `iface` / `socket` 划分，不直接 fork
- [ ] 与 `references` 包名、职责一一对照表（写在 `docs/adr/001-rust-layout.md`）

---
