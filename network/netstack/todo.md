# Todo List

本文件记录当前重写进度与下一步优先级（以 `references/` 为对照）。

## 已完成模块

- [x] **core**：`IPv4Address`、`Subnet`、`LinkAddress`、`kIPv4EmptySubnet`
- [x] **header**：`Checksum`、`IPv4Header`、`UDPHeader`
- [x] **stack 接口**：`registration.hh`、`transport.hh`
- [x] **stack 实现**：`NIC`、`Stack`（`TransportDispatcher`、`AddAddress`）、`TransportDemuxer`
- [x] **ports**：`PortManager`（简化 Reserve/Release）
- [x] **link**：`loopback`、`channel`
- [x] **net/ipv4**：入站 `HandlePacket`、出站 `SendPacket`
- [x] **transport/udp**：`Protocol`、`Endpoint`（Bind + echo）
- [x] **测试**：`header_ipv4`、`header_udp`、`stack_loopback`、`m0_ipv4_acceptance`、`m1_udp_echo`
- [x] **文档**：M0/M1 指南、ADR 001–003

## M0 可选后续

- [ ] `WritePackets` 批量写出
- [ ] 更完整的 `Stack` 统计与 Go `Stats` 字段对齐

## M2：TCP 单连接（简化）

实施指南：[`docs/m2.md`](docs/m2.md)

- [ ] `seqnum/` + `header/tcp`
- [ ] `transport/tcp/` 极简状态机（SYN → data → FIN）
- [ ] Demuxer TCP 分支（Listen + 四元组）
- [ ] 集成测试（channel link）
