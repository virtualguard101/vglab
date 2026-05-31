# 模块与参考实现对照

C++ 目录按 **职责** 拆分，不与 `references/` 包名一一对应；下表说明各组件应放在哪。

| C++ 路径 | 参考 (Go) | 职责 |
|----------|-----------|------|
| `include/netstack/address.hpp` | `tcpip.Address`（IPv4 四字节用法） | 地址解析、点分十进制、线上读写 |
| `include/netstack/subnet.hpp` | `tcpip.Subnet`、`NewSubnet` | 前缀/掩码、`Contains`；**`kIPv4EmptySubnet`（0.0.0.0/0）** |
| `include/netstack/header/checksum.hpp` | `header.Checksum` | RFC 1071 校验和（无状态） |
| `include/netstack/header/ipv4.hpp` | `header.IPv4`、`IPv4Fields`、偏移常量 | IPv4 头视图、`Encode`/`IsValid` |
| `src/netstack/*.cc` | `tcpip` 根包部分逻辑 | 上述类型的实现 |
| `src/header/*.cc` | `tcpip/header` | 报文头编解码实现 |
| `src/stack/`（待建） | `tcpip/stack` | Stack、NIC、路由、协议注册 |
| `src/link/`（待建） | `tcpip/link` | loopback、channel、tun |
| `src/net/ipv4/`（待建） | `tcpip/network/ipv4` | 协议状态机、分片、交付传输层 |

## 为何 `kIPv4EmptySubnet` 不在 `header/`？

Go 把它写在 `header/ipv4.go` 里仅为测试设置默认路由方便。语义上它是 **路由表的 destination 前缀**（`tcpip.Subnet`），属于栈/路由概念，因此 C++ 放在 `netstack/subnet.hpp`。

设置默认路由时使用：

```cpp
#include "netstack/subnet.hpp"
// route.destination = netstack::kIPv4EmptySubnet;
```
