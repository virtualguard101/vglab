# netstack

Netstack 的 C++ 教学实现。目录布局见 [`todo.md`](todo.md) 中「C++ 重写建议」一节。

## 目录说明

| 路径 | 职责 |
|------|------|
| `include/netstack/` | 对外公开头文件 |
| `src/header/` | 报文头解析/构造（无状态） |
| `src/buffer/` | 报文缓冲区 |
| `src/stack/` | Stack、NIC、路由、分发 |
| `src/link/` | loopback、channel、tun |
| `src/net/` | ipv4、arp、分片 |
| `src/transport/` | udp、tcp |
| `src/ports/` | 端口管理 |
| `src/seqnum/` | TCP 序号算术 |
| `examples/` | 示例程序 |
| `tests/` | 集成/单元测试 |
| `docs/adr/` | 架构决策记录 |

参考实现：[`references/`](references/)
