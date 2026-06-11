# ADR 005：M2+ Demuxer 四元组与 Connect

## 状态

已接受（M2+）

## 背景

M2 用单一 `Endpoint` + 本地端口 demux，无法同时 LISTEN 与多条 ESTABLISHED 连接，也不支持主动打开。

## 决策

1. **双表 demuxer**：`listen_endpoints_`（本地键）+ `connected_endpoints_`（四元组）；TCP 入站先查四元组。
2. **拆分类型**：`Listener`（常驻 LISTEN）+ `Connection`（单连接状态机）。
3. **半连接即登记四元组**：发 SYN-ACK 或 SYN 后立即 `RegisterConnectedEndpoint`，第三次握手 ACK 靠四元组路由。
4. **backlog**：Listener 限制 `connections_.size()`，超出丢弃 SYN（不发 RST）。
5. **端口归属**：`Connect()` 的 Connection `owns_port_`；被动 Connection 由 Listener 占端口。
6. **仍跳过**：FIN-WAIT-1/2、TIME-WAIT、RTO。

## 后果

- 可在同一端口上连续 Accept（连接由 `Listener` 持有生命周期）。
- 删除 `tcp::Endpoint`；测试迁移到 Listener/Connection。
- 与 gVisor `transport_demuxer` 方向一致，仍大幅简化。
