# 模块与参考实现对照

C++ 目录按 **职责** 拆分，不与 `references/` 包名一一对应；下表说明各组件应放在哪。

| C++ 路径 | 参考 (Go) | 职责 |
|----------|-----------|------|
| `include/netstack/address.hh` | `tcpip.Address` | IPv4 / 链路地址解析与格式化 |
| `include/netstack/subnet.hh` | `tcpip.Subnet`、`NewSubnet` | 前缀/掩码、`Contains`；**`kIPv4EmptySubnet`（0.0.0.0/0）** |
| `include/netstack/header/checksum.hh` | `header.Checksum` | RFC 1071 校验和 |
| `include/netstack/header/ipv4.hh` | `header.IPv4`、`IPv4Fields` | IPv4 头视图、`Encode` / `IsValid` / checksum |
| `src/netstack/*.cc` | `tcpip` 根包部分逻辑 | 地址、子网实现 |
| `src/header/*.cc` | `tcpip/header` | 报文头编解码 |
| `include/netstack/stack/registration.hh` | `tcpip/stack` 接口层 | `LinkEndpoint`、`NetworkDispatcher`、`PacketBuffer` |
| `include/netstack/stack/stack.hh` | `stack.Stack` | NIC / 协议注册、`GetStats` |
| `include/netstack/stack/nic.hh` | `stack.NIC` | `DeliverNetworkPacket`、RX 统计 |
| `include/netstack/stack/network_protocol.hh` | `stack.NetworkProtocol` | 网络层协议抽象、传输层分发 stub |
| `src/stack/*.cc` | `tcpip/stack` | Stack / NIC 实现 |
| `include/netstack/net/ipv4/protocol.hh` | `network/ipv4` | IPv4 `HandlePacket`（M0：无分片） |
| `src/net/ipv4/protocol.cc` | `network/ipv4` | 校验、剥头、交付传输层 |
| `include/netstack/link/loopback.hh` | `link/loopback` | 环回 `WritePacket` → dispatcher |
| `include/netstack/link/channel.hh` | `link/channel` | 测试用注入 / 出站队列 |
| `src/link/*.cc` | `tcpip/link` | loopback、channel 实现 |

## 测试布局

| 路径 | 说明 |
|------|------|
| `tests/header/ipv4_test.cc` | IPv4 头与 checksum 单元测试 |
| `tests/stack/loopback_dispatch_test.cc` | loopback / channel 基本分发 |
| `tests/m0/ipv4_acceptance_test.cc` | M0 验收（合法/坏 checksum/短包/未知协议） |
