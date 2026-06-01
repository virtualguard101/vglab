# Todo List

本文件记录当前重写进度与下一步优先级（以 `references/` 为对照）。

## 已完成模块

- [x] **core**：`IPv4Address`、`Subnet`、`LinkAddress`、`kIPv4EmptySubnet`
- [x] **header**：`Checksum`、`IPv4Header`
- [x] **stack 接口**：`registration.hh`（`NetworkDispatcher`、`LinkEndpoint`、`InjectableLinkEndpoint`）
- [x] **stack 实现**：`NIC`（`DeliverNetworkPacket`）、`Stack`（协议/NIC 注册、`GetStats`）
- [x] **link**：`loopback`、`channel`（`InjectInbound` / 出站队列）
- [x] **net/ipv4**：`Protocol::HandlePacket`（校验 + 剥头 + 交付 transport stub）
- [x] **测试**：`header_ipv4`、`stack_loopback`、`m0_ipv4_acceptance`
- [x] **M0 文档**：`docs/module-map.md`、`docs/adr/001-buffer-ownership.md`、`docs/adr/002-m0-scope.md`

## M0 可选后续

- [ ] `WritePackets` 批量写出（当前默认返回 `NotSupported`）
- [ ] 更完整的 `Stack` 统计与 Go `Stats` 字段对齐

## M1：UDP echo

实施指南：[`docs/m1.md`](docs/m1.md)

- [ ] `src/ports/`
- [ ] `src/transport/udp/`
- [ ] UDP echo 集成测试（channel link）
