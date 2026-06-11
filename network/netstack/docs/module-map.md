# 模块与参考实现对照

C++ 目录按 **职责** 拆分，不与 `references/` 包名一一对应；下表说明各组件应放在哪。

| C++ 路径 | 参考 (Go) | 职责 |
|----------|-----------|------|
| `include/netstack/address.hh` | `tcpip.Address` | IPv4 / 链路地址解析与格式化 |
| `include/netstack/subnet.hh` | `tcpip.Subnet`、`NewSubnet` | 前缀/掩码、`Contains`；**`kIPv4EmptySubnet`（0.0.0.0/0）** |
| `include/netstack/header/checksum.hh` | `header.Checksum` | RFC 1071 校验和 |
| `include/netstack/header/ipv4.hh` | `header.IPv4`、`IPv4Fields` | IPv4 头视图、`Encode` / `IsValid` / checksum |
| `include/netstack/header/udp.hh` | `header.UDP` | UDP 头视图、`Encode` / `ParsePorts` |
| `include/netstack/header/tcp.hh` | `header.TCP` | TCP 头视图、Flags、`Encode` / `ParsePorts` |
| `include/netstack/seqnum.hh` | `tcpip/seqnum` | 序列号算术（回绕安全比较） |
| `include/netstack/stack/transport.hh` | `stack.Transport*`、`transportDemuxer` | 传输层接口与 demuxer |
| `include/netstack/ports/port_manager.hh` | `ports.PortManager` | 端口占用（M1 简化） |
| `include/netstack/transport/udp/*.hh` | `transport/udp` | UDP 协议与 echo endpoint |
| `include/netstack/transport/tcp/*.hh` | `transport/tcp` | TCP 协议、`Listener`、`Connection` |
| `include/netstack/net/ipv4/send.hh` | `network/ipv4` 出站 | IPv4 封装 + `WritePacket` |
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
| `include/netstack/link/tun.hh` | `link/tun` | 打开 TUN 设备（M3，`NETSTACK_ENABLE_TUN`） |
| `include/netstack/link/fdbased.hh` | `link/fdbased` | 基于 FD 的 `LinkEndpoint`（M3） |
| `src/link/*.cc` | `tcpip/link` | loopback、channel；M3 追加 tun、fdbased |
| `examples/tun_tcp_echo.cc` | `sample/tun_tcp_echo` | M3 宿主 `nc` 验收 demo |

## 测试布局

| 路径 | 说明 |
|------|------|
| `tests/header/ipv4_test.cc` | IPv4 头与 checksum 单元测试 |
| `tests/stack/loopback_dispatch_test.cc` | loopback / channel 基本分发 |
| `tests/m0/ipv4_acceptance_test.cc` | M0 验收（合法/坏 checksum/短包/未知协议） |
| `tests/header/udp_test.cc` | UDP 头单测 |
| `tests/m1/udp_echo_test.cc` | M1 验收（channel 注入 → echo → 出站队列） |
| `tests/header/tcp_test.cc` | TCP 头单测 |
| `tests/seqnum/seqnum_test.cc` | 序列号算术单测 |
| `tests/m2/tcp_handshake_test.cc` | M2 三次握手验收 |
| `tests/m2/tcp_transfer_test.cc` | M2 数据传输与 FIN 关闭 |
| `docs/tcp-rfc793-states.md` | RFC 793 Figure 6 与 `TcpState` 对照 |
| `docs/m2+.md` | M2+ 四元组 demux、Connect、backlog |
| `docs/m3.md` | M3 TUN 对接宿主实施指南 |
| `docs/adr/006-m3-tun-linux.md` | M3：Linux 裸 IPv4、CI 策略 |
| `tests/m2/tcp_connect_test.cc` | M2+ Connect 主动打开 |
| `tests/m2/tcp_backlog_test.cc` | M2+ Listener backlog |
| `tests/link/fdbased_test.cc` | M3 可选：pipe 模拟 FD（非默认 ctest） |
