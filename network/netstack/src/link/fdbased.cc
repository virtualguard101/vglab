/**
 * @file fdbased.cc
 * @brief FD 链路 endpoint：read 入站 / write 出站（TUN 裸 IPv4）。
 *
 * ## 与 channel 的差异
 *
 * - channel：测试 InjectInbound；出站进 outbound_ 队列。
 * - fdbased：入站由 OS read；出站 write 到 TUN，宿主可见。
 *
 * @see include/netstack/link/fdbased.hh
 * @see docs/m3.md
 */

#include "netstack/link/fdbased.hh"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <vector>

#include "netstack/header/ipv4.hh"

namespace netstack::link {

namespace {

using stack::ErrorCode;
using stack::PacketBuffer;
using stack::StackError;
using stack::StackResult;

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

}  // namespace

FdEndpoint::FdEndpoint(int fd, uint32_t mtu, LinkAddress link_addr)
    : fd_(fd), mtu_(mtu), link_addr_(link_addr) {
  read_buf_.resize(mtu > 0 ? mtu : 1500);
  if (fd_ >= 0) {
    const int fl = ::fcntl(fd_, F_GETFL, 0);
    if (fl >= 0) {
      (void)::fcntl(fd_, F_SETFL, fl | O_NONBLOCK);
    }
  }
}

FdEndpoint::~FdEndpoint() {
  if (fd_ >= 0) {
    ::close(fd_);
    fd_ = -1;
  }
}

void FdEndpoint::PollOnce() {
  if (dispatcher_ == nullptr || fd_ < 0) {
    return;
  }

  while (true) {
    const ssize_t n = ::read(fd_, read_buf_.data(), read_buf_.size());
    if (n < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      }
      break;
    }
    if (n == 0) {
      break;
    }

    std::vector<uint8_t> buf(read_buf_.begin(),
                             read_buf_.begin() + static_cast<size_t>(n));
    dispatcher_->DeliverNetworkPacket(*this, LinkAddress{}, LinkAddress{},
                                      header::kIPv4ProtocolNumber,
                                      PacketBuffer(std::move(buf)));
  }
}

uint32_t FdEndpoint::MTU() const { return mtu_; }

stack::LinkEndpointCapabilities FdEndpoint::Capabilities() const { return 0; }

uint16_t FdEndpoint::MaxHeaderLength() const { return 0; }

LinkAddress FdEndpoint::LinkAddr() const { return link_addr_; }

StackResult FdEndpoint::WritePacket(stack::Route* route, const stack::GSO* gso,
                                    stack::NetworkProtocolNumber protocol,
                                    PacketBuffer pkt) {
  (void)route;
  (void)gso;
  (void)protocol;
  if (fd_ < 0) {
    return StackError{ErrorCode::kInvalidEndpointState};
  }

  PacketBuffer outbound = MergeHeaderAndData(std::move(pkt));
  const auto& data = outbound.Data();
  if (data.empty()) {
    return std::nullopt;
  }

  const ssize_t n = ::write(fd_, data.data(), data.size());
  if (n < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return StackError{ErrorCode::kInvalidEndpointState};
    }
    return StackError{ErrorCode::kInvalidEndpointState};
  }
  if (static_cast<size_t>(n) != data.size()) {
    return StackError{ErrorCode::kInvalidEndpointState};
  }
  return std::nullopt;
}

void FdEndpoint::Attach(stack::NetworkDispatcher* dispatcher) {
  dispatcher_ = dispatcher;
}

bool FdEndpoint::IsAttached() const { return dispatcher_ != nullptr; }

}  // namespace netstack::link
