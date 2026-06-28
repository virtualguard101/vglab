# Todo List

本文件记录当前重写进度与下一步优先级（以 `references/` 为对照）。

## 已完成模块

- [x] **core**：`IPv4Address`、`Subnet`、`LinkAddress`、`kIPv4EmptySubnet`
- [x] **header**：`Checksum`、`IPv4Header`、`UDPHeader`、`TCPHeader`
- [x] **seqnum**：`LessThan`、`Add`、`InWindow`（`include/netstack/seqnum.hh`）
- [x] **stack 接口**：`registration.hh`、`transport.hh`
- [x] **stack 实现**：`NIC`、`Stack`（`TransportDispatcher`、`AddAddress`）、`TransportDemuxer`
- [x] **ports**：`PortManager`（简化 Reserve/Release）
- [x] **link**：`loopback`、`channel`
- [x] **net/ipv4**：入站 `HandlePacket`、出站 `SendPacket`
- [x] **transport/udp**：`Protocol`、`Endpoint`（Bind + echo）
- [x] **transport/tcp**：`Protocol`、`Listener`、`Connection`（Listen/Connect + 握手 + 数据 + FIN）
- [x] **测试**：…、`m2_tcp_connect`、`m2_tcp_backlog`（共 11 项 ctest）
- [x] **文档**：M0/M1/M2/M2+/M3 指南、ADR 001–006

## M0 可选后续

- [ ] `WritePackets` 批量写出
- [ ] 更完整的 `Stack` 统计与 Go `Stats` 字段对齐

## M2 / M2+：TCP

- [x] M2 基础：[`docs/m2.md`](docs/m2.md)（`ctest -R m2_tcp`）
- [x] M2+ 扩展：[`docs/m2+.md`](docs/m2+.md) — 四元组 demux、Connect、Listener backlog

## M2+ 之后 / M3

- [ ] RTO、重传、拥塞控制
- [ ] FIN-WAIT / TIME-WAIT 完整路径

## M3：TUN 对接宿主

实施指南：[`docs/m3.md`](docs/m3.md)（ADR [`006`](docs/adr/006-m3-tun-linux.md)）

- [x] `link/tun`：`OpenTun`（`IFF_TUN | IFF_NO_PI`、非阻塞）
- [x] `link/fdbased`：`FdEndpoint`（`PollOnce` + `WritePacket`）
- [x] `examples/tun_tcp_echo`（`NETSTACK_ENABLE_TUN=ON`）
- [ ] 宿主机 `ip tuntap` + `nc` 手动验收
- [x] `tests/link/fdbased_test.cc`（`NETSTACK_TUN_INTEGRATION_TEST=ON`）
- [x] channel ctest 11 项仍全绿（`NETSTACK_ENABLE_TUN=OFF` 默认）
