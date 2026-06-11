# ADR 004：M2 TCP 简化语义

## 状态

已接受（M2）

## 背景

M2 目标是在 channel link 上跑通单连接的三次握手、按序单段数据与 FIN 关闭，教学栈不追求与 Linux/gVisor 行为逐字段对齐。

## 决策

1. **固定窗口** 65535，不做动态窗口调整与拥塞控制。
2. **无 RTO / 重传**；测试环境不丢包，乱序段直接丢弃。
3. **无 TCP 选项**；头固定 20 字节（`DataOffset == 5`）。
4. **被动打开优先**：`Listen` + 测试注入对端段；`Connect` 推迟。
5. **TIME_WAIT**：收到对端 ACK 我方 FIN 后立即 `CLOSED`，不等 2MSL。
6. **校验和**：出站计算 TCP 伪首部校验和；入站 **不验证** TCP checksum（与 M1 UDP 测试一致）。
7. **Demuxer**：M2 单连接场景沿用 `(local_ip, local_port)` 精确匹配；多连接 / Listen backlog 推迟。

## 后果

- 实现与测试更简单，可专注状态机与 seqnum 教学。
- 不可直接用于有损网络或生产语义；RTO、SACK、多连接在 M2+ 扩展。

## 状态图对照

RFC 793 Figure 6 与 M2 已实现转移的完整对照见
[`docs/tcp-rfc793-states.md`](../tcp-rfc793-states.md)。
