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
- [x] **transport/tcp**：`Protocol`、`Endpoint`（Listen + 握手 + 数据 + FIN）
- [x] **测试**：`header_ipv4`、`header_udp`、`header_tcp`、`seqnum`、`stack_loopback`、`m0_ipv4_acceptance`、`m1_udp_echo`、`m2_tcp_handshake`、`m2_tcp_transfer`
- [x] **文档**：M0/M1/M2 指南、ADR 001–004

## M0 可选后续

- [ ] `WritePackets` 批量写出
- [ ] 更完整的 `Stack` 统计与 Go `Stats` 字段对齐

## M2：TCP 单连接（简化）

实施指南：[`docs/m2.md`](docs/m2.md) — **验收已完成**（`ctest -R m2_tcp`）

- [x] `seqnum` + `header/tcp`
- [x] `transport/tcp/` 极简状态机（SYN → data → FIN）
- [x] 集成测试（channel link）
- [ ] Demuxer TCP 四元组 / Listen backlog（M2+，单连接场景已够用）
- [ ] `Connect` 主动打开（可选扩展）
