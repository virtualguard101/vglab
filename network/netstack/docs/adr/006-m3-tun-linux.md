# ADR 006：M3 TUN（Linux 裸 IPv4）

## 状态

已接受（M3 文档阶段）

## 背景

M0–M2+ 全部通过 **channel** 注入/断言完成集成测。M3 需将同一协议栈接到 Linux **TUN** 设备，用宿主机 `nc` 验收，但不能破坏无 root 的 CI。

## 决策

1. **平台**：仅 **Linux**（`ioctl`、`/dev/net/tun`）；其他 OS 不实现。
2. **模式**：**TUN + `IFF_NO_PI`**，读写 **裸 IPv4**（无 4 字节 PI 头），与 channel `MaxHeaderLength()==0` 一致。
3. **TAP**：在 `docs/m3.md` 中描述完整路径；**实现推迟**到 M3+（需 L2）。
4. **链路实现**：精简 `FdEndpoint`（单 FD、`read`/`write`、`PollOnce`），不对标完整 `fdbased`（无 MMAP/FANOUT）。
5. **事件模型**：demo 显式 `PollOnce` + 轮询 `Accept`/`Read`；**不**引入 `waiter`。
6. **路由**：M3 **不实现** `SetRouteTable`；假定单 NIC、出站写同一 TUN。
7. **ARP/ICMP/IPv6**：TUN 裸 IP 场景不注册；参考 demo 的 ARP 分支不移植。
8. **CMake**：`NETSTACK_ENABLE_TUN` 默认 **OFF**；ctest **不默认** 跑需 root 的 TUN 测试。
9. **Demo API**：使用现有 `tcp::Listener` + `tcp::Connection`，不用 `NewEndpoint`+`waiter`。

## 后果

- 开发者需 Linux + 手动配置 `ip tuntap` 才能验 M3；CI 仍只靠 channel。
- 与 gVisor `fdbased` 行为子集一致，便于对照阅读参考代码。
- TAP/L2 、多 NIC 路由成为明确后续项，避免 M3 范围膨胀。

## 相关

- [`docs/m3.md`](../m3.md)
- [`docs/adr/004-m2-tcp-simplified.md`](004-m2-tcp-simplified.md)
