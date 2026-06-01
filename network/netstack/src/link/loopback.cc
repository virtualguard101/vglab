/**
 * @file loopback.cc
 * @brief 环回链路实现（references/tcpip/link/loopback）。
 *
 * ## 行为摘要
 *
 * - **WritePacket**：不经过真实网卡，立即以入站身份调用
 *   `NetworkDispatcher::DeliverNetworkPacket`（对标 loopback.go 约 77–91 行）。
 * - **WriteRawPacket**：解析 14 字节以太网头中的 EtherType，再交付网络层（M0
 * 少测）。
 * - MTU 65536；MaxHeaderLength()=0，表示 M0 测试可直接写裸 IPv4。
 *
 * @see include/netstack/link/loopback.hh
 */

#include "netstack/link/loopback.hh"

#include <vector>

namespace netstack::link {

namespace {

constexpr uint32_t kLoopbackMTU = 65536;
constexpr size_t kEthernetMinimumSize = 14;

using stack::ErrorCode;
using stack::LinkEndpointCapability;
using stack::PacketBuffer;
using stack::StackError;
using stack::StackResult;

/**
 * @brief 出站前若 Header 与 Data 分离，合并为单 buffer 再入站。
 *
 * 参考实现中 loopback 也会整理 buffer 视图，避免 NIC 侧看到分裂的存储。
 */
PacketBuffer MergeHeaderAndData(PacketBuffer pkt) {
  if (pkt.Header().empty()) {
    return pkt;
  }
  std::vector<uint8_t> merged;
  merged.reserve(pkt.Header().size() + pkt.Data().size());
  merged.insert(merged.end(), pkt.Header().begin(), pkt.Header().end());
  merged.insert(merged.end(), pkt.Data().begin(), pkt.Data().end());
  return PacketBuffer(std::move(merged));
}

class LoopbackEndpoint : public stack::LinkEndpoint {
 public:
  uint32_t MTU() const override { return kLoopbackMTU; }

  stack::LinkEndpointCapabilities Capabilities() const override {
    return LinkEndpointCapability::kRXChecksumOffload |
           LinkEndpointCapability::kTXChecksumOffload |
           LinkEndpointCapability::kSaveRestore |
           LinkEndpointCapability::kLoopback;
  }

  uint16_t MaxHeaderLength() const override { return 0; }

  LinkAddress LinkAddr() const override { return LinkAddress{}; }

  /**
   * @brief 环回发送：合并 header+data 后立刻 DeliverNetworkPacket。
   *
   * @param protocol 网络层协议号（IPv4 写 kIPv4ProtocolNumber）。
   * @param pkt Data 通常为完整 IPv4 报文（M0 测试约定）。
   */
  StackResult WritePacket(stack::Route* route, const stack::GSO* gso,
                          stack::NetworkProtocolNumber protocol,
                          PacketBuffer pkt) override {
    (void)route;
    (void)gso;
    if (dispatcher_ == nullptr) {
      return StackError{ErrorCode::kInvalidEndpointState};
    }
    PacketBuffer inbound = MergeHeaderAndData(std::move(pkt));
    dispatcher_->DeliverNetworkPacket(*this, LinkAddress{}, LinkAddress{},
                                      protocol, std::move(inbound));
    return std::nullopt;
  }

  /**
   * @brief 发送已含以太网头的帧：剥离 14 字节 L2，按 EtherType 上交网络层。
   */
  StackResult WriteRawPacket(std::vector<uint8_t> frame) override {
    if (dispatcher_ == nullptr) {
      return StackError{ErrorCode::kInvalidEndpointState};
    }
    if (frame.size() < kEthernetMinimumSize) {
      return StackError{ErrorCode::kBadAddress};
    }
    const uint16_t ethertype =
        (static_cast<uint16_t>(frame[12]) << 8) | frame[13];
    std::vector<uint8_t> link_hdr(frame.begin(),
                                  frame.begin() + kEthernetMinimumSize);
    frame.erase(frame.begin(), frame.begin() + kEthernetMinimumSize);

    PacketBuffer pkt(std::move(frame));
    pkt.LinkHeader() = std::move(link_hdr);
    dispatcher_->DeliverNetworkPacket(*this, LinkAddress{}, LinkAddress{},
                                      ethertype, std::move(pkt));
    return std::nullopt;
  }

  void Attach(stack::NetworkDispatcher* dispatcher) override {
    dispatcher_ = dispatcher;
  }

  bool IsAttached() const override { return dispatcher_ != nullptr; }

 private:
  stack::NetworkDispatcher* dispatcher_{nullptr};
};

}  // namespace

std::unique_ptr<stack::LinkEndpoint> NewLoopback() {
  return std::make_unique<LoopbackEndpoint>();
}

}  // namespace netstack::link
