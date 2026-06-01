# ADR 002: M0 范围边界

## 状态

已接受

## 包含

- loopback + channel 链路
- NIC `DeliverNetworkPacket` → IPv4 `HandlePacket`
- IPv4 头 `IsValid`、**接收路径 checksum 校验**、剥头
- `RecordingTransportDispatcher` 观测交付
- 单元/集成测试：合法帧、坏 checksum、短包、未知 L3 协议号

## 不包含

- UDP/TCP、端口、socket API
- 分片重组、转发、ARP、TUN
- 以太网 `WriteRawPacket` 完整路径（loopback 可 stub）
- 与 Linux 内核逐字段对齐

## 测试约定

入站测试帧 **无以太网头**（`MaxHeaderLength() == 0`），由 `channel::InjectInbound` 或 loopback `WritePacket` 直接递交 L3 载荷。
