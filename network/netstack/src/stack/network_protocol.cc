/**
 * @file network_protocol.cc
 * @brief 网络层相关辅助实现（M0：测试用 TransportDispatcher）。
 *
 * - 生产路径中，IPv4 剥头后会调用
 * `TransportDispatcher::DeliverTransportPacket`。
 * - M0 尚未实现真实 UDP/TCP，故提供 `RecordingTransportDispatcher`：
 *   仅记录「交付了哪一层协议号、载荷多大」，供集成测试断言。
 * - M1 将用 `Stack` 或 `TransportDemuxer` 替换该 stub（见 docs/m1.md）。
 *
 * @see include/netstack/stack/network_protocol.hh
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
