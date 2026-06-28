/**
 * @file network_protocol.cc
 * @brief 网络层相关辅助实现（M0：测试用 TransportDispatcher）。
 *
 * ## 分层接口
 *
 * - 生产路径中，IPv4 剥头后会调用
 *   `TransportDispatcher::DeliverTransportPacket`。
 * - `NetworkProtocol` 通过 `SetTransportDispatcher` 持有非拥有指针。
 *
 * ## RecordingTransportDispatcher
 *
 * M0/M1 集成测试在尚未接线真实 UDP demuxer 时，用本桩验证：
 * - IPv4 `HandlePacket` 是否调用了交付；
 * - 上层协议号（如 17）与载荷长度是否符合预期。
 *
 * 不保存 payload 内容，仅记 `(protocol, size)`，降低断言成本。
 * M1 起由 `Stack` / `TransportDemuxer` 替换（见 docs/m1.md）。
 *
 * @see include/netstack/stack/network_protocol.hh
 * @see docs/m0.md
 */

#include "netstack/stack/network_protocol.hh"

namespace netstack::stack {

/**
 * @brief 测试桩：把每次传输层交付记入 entries_，不真正处理 UDP/TCP。
 *
 * @param route 当前路由上下文（M0 测试可忽略）。
 * @param protocol IP 头 Protocol 字段（如 17=UDP）。
 * @param pkt 已剥 IPv4 头的载荷；只统计 Data().size()。
 */
void RecordingTransportDispatcher::DeliverTransportPacket(
    Route* route, TransportProtocolNumber protocol, PacketBuffer pkt) {
  (void)route;
  entries_.push_back(Entry{protocol, pkt.Data().size()});
}

}  // namespace netstack::stack
