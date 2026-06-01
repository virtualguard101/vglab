/**
 * @file loopback.cc
 * @brief 环回链路实现（references/tcpip/link/loopback）。
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

/** @brief 将 Header + Data 合并到 Data（loopback 回环入站用）。 */
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

  StackResult WriteRawPacket(std::vector<uint8_t> frame) override {
    if (dispatcher_ == nullptr) {
      return StackError{ErrorCode::kInvalidEndpointState};
    }
    if (frame.size() < kEthernetMinimumSize) {
      return StackError{ErrorCode::kBadAddress};
    }
    // 以太网类型字段在字节 12-13（简化的 EtherType 解析）。
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
