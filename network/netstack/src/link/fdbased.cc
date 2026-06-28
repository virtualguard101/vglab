/**
 * @file fdbased.cc
 * @brief FD 链路 endpoint：read 入站 / write 出站（TUN 裸 IPv4）。
 *
 * ## 在数据路径中的位置（M3）
 *
 * @code
 * 入站: read(fd) → PollOnce → NIC::DeliverNetworkPacket(0x0800)
 * 出站: ipv4::SendPacket → WritePacket → write(fd)
 * @endcode
 *
 * ## 与 channel / loopback 对比
 *
 * | 链路 | 入站驱动 | 出站 |
 * |------|----------|------|
 * | channel | 测试 InjectInbound | 进 outbound_ 队列 |
 * | loopback | WritePacket 立即环回 | 同左 |
 * | fdbased | OS read + PollOnce | write 到 TUN，宿主可见 |
 *
 * 对标 references/tcpip/link/fdbased 的 **Readv + dispatch** 子集；
 * 无 FANOUT、MMAP、多 FD。
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

/**
 * @brief 出站前合并 Header 与 Data（与 loopback 相同约定）。
 *
 * IPv4::SendPacket 通常只填 Data；若未来 L2 prepend 到 Header，
 * 写 TUN 前须合成连续 buffer。
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

}  // namespace

/**
 * @brief 接管 fd 所有权；预分配 read_buf_；强制非阻塞。
 *
 * socketpair 单测也依赖 O_NONBLOCK：PollOnce 在读完一批包后须以
 * EAGAIN 结束循环，否则会阻塞在第二次 read。
 */
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

/**
 * @brief 非阻塞读循环：直到 EAGAIN 或错误。
 *
 * 每读一帧即调用 `dispatcher_->DeliverNetworkPacket`。
 * TUN 裸 IP 时 protocol 固定为 `kIPv4ProtocolNumber` (0x0800)。
 *
 * **调用方**：demo 主循环在 poll 可读后调用；不另起线程。
 */
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

/** @brief TUN 裸 IP，与 channel 一致，上层无需预留 L2 头。 */
uint16_t FdEndpoint::MaxHeaderLength() const { return 0; }

LinkAddress FdEndpoint::LinkAddr() const { return link_addr_; }

/**
 * @brief 将完整 IPv4 报文写入 fd。
 *
 * 非阻塞 write：EAGAIN 返回错误（demo 可下次重试；M3 未实现发送队列）。
 */
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

/** @brief NIC 构造时调用；Attach 后 PollOnce 才能交付入站包。 */
void FdEndpoint::Attach(stack::NetworkDispatcher* dispatcher) {
  dispatcher_ = dispatcher;
}

bool FdEndpoint::IsAttached() const { return dispatcher_ != nullptr; }

}  // namespace netstack::link
