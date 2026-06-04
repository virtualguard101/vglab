# ADR 003: M1 UDP echo 范围

## 状态

已接受

## 决策

- `Stack` 实现 `TransportDispatcher`，IPv4 注册时默认接入（不覆盖已有测试桩）。
- UDP RX **不验证** UDP 校验和（checksum 字段写 0）。
- Echo 在 `HandlePacket` 内**同步**回射，经 `net::ipv4::SendPacket` 出站。
- 测试以 **channel 出站队列** 断言，不实现 `Read()`/waiter。

## 后果

- M0 测试可继续手动挂载 `RecordingTransportDispatcher`。
- M2 再引入完整 socket 与非阻塞模型。
