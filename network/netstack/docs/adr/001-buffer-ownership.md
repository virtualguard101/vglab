# ADR 001: M0 报文缓冲区所有权

## 状态

已接受（M0）

## 背景

Go 参考实现使用 `stack.PacketBuffer` 链式缓冲；M0 需要可调试、可单测的最小路径。

## 决策

- M0 使用 `std::vector<uint8_t>` 作为 `PacketBuffer` 载体，按值移动传递。
- 网络层 `HandlePacket` 在剥头时 `resize` / 拷贝 payload，不引入链式 buffer。
- 出站 `WritePacket` 取得 buffer 所有权；失败路径由调用方决定是否重试（M0 不实现重传）。

## 后果

- 实现简单，与 smoltcp「先 vector 再演进」一致。
- 后续 M1+ 若需零拷贝或 GSO，可再引入 `PacketBuffer` 视图或池化，需新 ADR。
