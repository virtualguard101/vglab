/**
 * @file registration.hh
 * @brief stack 模块的核心接口（对标 references/tcpip/stack/registration.go）。
 *
 * Go 版通过 interface 定义 link / network / transport 之间的可插拔边界；
 * C++ 版用纯虚基类表达相同语义，便于运行时注册 loopback、ipv4 等实现。
 *
 * @see references/tcpip/stack/registration.go
 * @see docs/module-map.md
 *
 * ## M0 入站数据路径（阅读本文件时的主线）
 *
 * @code
 * LinkEndpoint          NetworkDispatcher (NIC)     NetworkProtocol
 * WritePacket/Inject --> DeliverNetworkPacket -----> HandlePacket (ipv4)
 *                                                      |
 *                                                      v
 *                                            TransportDispatcher (stub/UDP)
 * @endcode
 *
 * - **PacketBuffer**：教学版用 vector 承载；move 传递所有权（见 adr/001）。
 * - **StackResult**：std::nullopt 表示成功，对标 Go 的 nil *tcpip.Error。
 * - **LinkEndpoint::Attach**：把 NIC（实现 NetworkDispatcher）注册为入站回调。
 */

#pragma once

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include "netstack/address.hh"

namespace netstack::stack {

/** @brief L3 网络协议号类型，2字节无符号整数（如以太网 EtherType 0x0800 =
 * IPv4）。*/
using NetworkProtocolNumber = uint16_t;

/** @brief 传输层协议号类型，单字节无符号整数（如 6 = TCP，17 = UDP）。*/
using TransportProtocolNumber = uint8_t;

/** @brief NIC 标识类型，4字节无符号整数（对标 tcpip.NICID）。*/
using NICID = uint32_t;

/**
 * @brief 可变长网络层地址（对标 tcpip.Address）。
 *
 * IPv4 为 4 字节；IPv6 为 16 字节。教学栈在 M0 阶段以 IPv4 为主，
 * 路由/endpoint 侧统一用此类型以保持与参考实现一致。
 */
using Address = std::vector<uint8_t>;

class LinkEndpoint;

// ---------------------------------------------------------------------------
// 错误与能力
// ---------------------------------------------------------------------------

/**
 * @brief 协议栈操作错误码（子集，随实现逐步补齐）。
 *
 * Go 返回 `*tcpip.Error`，nil 表示成功；C++ 用 std::nullopt 表示成功。
 */
enum class ErrorCode {
  kOk = 0,
  kInvalidEndpointState,
  kNotSupported,
  kBadAddress,
};

struct StackError {
  ErrorCode code{ErrorCode::kOk};

  friend bool operator==(const StackError& a, const StackError& b) {
    return a.code == b.code;
  }
};

/** @brief 成功为 std::nullopt；失败为 StackError。 */
using StackResult = std::optional<StackError>;

/**
 * @brief 链路层 endpoint 能力位集（对标 LinkEndpointCapabilities）。
 *
 * @see references/tcpip/stack/registration.go (Capability*)
 */
enum class LinkEndpointCapability : uint32_t {
  kNone = 0,
  kTXChecksumOffload = 1u << 0,
  kRXChecksumOffload = 1u << 1,
  kResolutionRequired = 1u << 2,
  kSaveRestore = 1u << 3,
  kDisconnectOk = 1u << 4,
  kLoopback = 1u << 5,
  kHardwareGSO = 1u << 6,
  kSoftwareGSO = 1u << 7,
};

using LinkEndpointCapabilities = uint32_t;

constexpr LinkEndpointCapabilities operator|(LinkEndpointCapability a,
                                             LinkEndpointCapability b) {
  return static_cast<LinkEndpointCapabilities>(static_cast<uint32_t>(a) |
                                               static_cast<uint32_t>(b));
}

constexpr LinkEndpointCapabilities operator|(LinkEndpointCapabilities a,
                                             LinkEndpointCapability b) {
  return a | static_cast<LinkEndpointCapabilities>(static_cast<uint32_t>(b));
}

constexpr LinkEndpointCapabilities operator&(LinkEndpointCapabilities a,
                                             LinkEndpointCapability b) {
  return a & static_cast<LinkEndpointCapabilities>(static_cast<uint32_t>(b));
}

// ---------------------------------------------------------------------------
// GSO / 分片描述符（WritePackets 用，M1+ 可深化）
// ---------------------------------------------------------------------------

enum class GSOType : uint8_t {
  kNone = 0,
  kTCPv4,
  kTCPv6,
  kUDP,
  kSW,  ///< 软件 GSO，经 WritePackets 发送
};

/**
 * @brief 通用分段卸载属性（对标 stack.GSO）。
 */
struct GSO {
  GSOType type{GSOType::kNone};
  bool needs_csum{false};
  uint16_t csum_offset{0};
  uint16_t mss{0};
  uint16_t l3_hdr_len{0};
  uint32_t max_size{0};
};

/**
 * @brief 批量发包时的包头描述（对标 stack.PacketDescriptor）。
 *
 * 参考实现中 Hdr 为 buffer.Prependable；教学版用 vector 承载可预留的 L2/L3
 * 头空间。
 */
struct PacketDescriptor {
  std::vector<uint8_t> hdr{};
  int off{0};
  int size{0};
};

// ---------------------------------------------------------------------------
// Route（WritePacket 出站路径，M0 最小字段集）
// ---------------------------------------------------------------------------

/**
 * @brief 经协议栈到某一目的地的出站路由（对标 stack.Route 的常用字段）。
 *
 * 完整 Route 在参考实现中还持有 network endpoint 引用与 PacketLooping；
 * M0 先暴露地址与链路地址，loopback 实现可忽略大部分字段。
 */
struct Route {
  Address remote_address{};
  LinkAddress remote_link_address{};
  Address local_address{};
  LinkAddress local_link_address{};
  Address next_hop{};
  NetworkProtocolNumber net_proto{0};
};

// ---------------------------------------------------------------------------
// PacketBuffer
// ---------------------------------------------------------------------------

/**
 * @brief 网络包缓冲区（对标 tcpip.PacketBuffer，简化版）。
 *
 * - Data：载荷；入站时常含尚未剥离的上层头。
 * - Header：出站时各层向前 prepend 的预留区（后续可用专用 Prependable 类型）。
 * - Link/Network/TransportHeader：可指向 Data/Header
 * 子区间的“视图”；教学版先用独立 vector。
 *
 * 所有权通过 move-only 语义转移（DeliverNetworkPacket / WritePacket）。
 */
class PacketBuffer {
 public:
  PacketBuffer() = default;
  explicit PacketBuffer(std::vector<uint8_t> data) : data_(std::move(data)) {}

  PacketBuffer(const PacketBuffer&) = delete;
  PacketBuffer& operator=(const PacketBuffer&) = delete;
  PacketBuffer(PacketBuffer&&) noexcept = default;
  PacketBuffer& operator=(PacketBuffer&&) noexcept = default;
  ~PacketBuffer() = default;

  std::vector<uint8_t>& Data() { return data_; }
  const std::vector<uint8_t>& Data() const { return data_; }

  std::vector<uint8_t>& Header() { return header_; }
  const std::vector<uint8_t>& Header() const { return header_; }

  std::vector<uint8_t>& LinkHeader() { return link_header_; }
  const std::vector<uint8_t>& LinkHeader() const { return link_header_; }

  std::vector<uint8_t>& NetworkHeader() { return network_header_; }
  const std::vector<uint8_t>& NetworkHeader() const { return network_header_; }

  std::vector<uint8_t>& TransportHeader() { return transport_header_; }
  const std::vector<uint8_t>& TransportHeader() const {
    return transport_header_;
  }

 private:
  std::vector<uint8_t> data_{};
  std::vector<uint8_t> header_{};
  std::vector<uint8_t> link_header_{};
  std::vector<uint8_t> network_header_{};
  std::vector<uint8_t> transport_header_{};
};

// ---------------------------------------------------------------------------
// NetworkDispatcher
// ---------------------------------------------------------------------------

/**
 * @brief 链路层入站后，将包交给网络层的分发器（对标 NetworkDispatcher）。
 */
class NetworkDispatcher {
 public:
  virtual ~NetworkDispatcher() = default;

  /**
   * @brief 将入站包交给协议栈进一步处理。
   *
   * @param link_ep 收到该包的链路 endpoint。
   * @param remote 远端链路地址；loopback 等可为默认构造的 LinkAddress。
   * @param local 本端链路地址；可为空。
   * @param protocol 网络层协议号（如 IPv4 0x0800）。
   * @param pkt 包缓冲区；调用后所有权转移给实现方。
   *
   * @note pkt.LinkHeader() 可能为空（loopback 无 L2 头）。
   */
  virtual void DeliverNetworkPacket(LinkEndpoint& link_ep, LinkAddress remote,
                                    LinkAddress local,
                                    NetworkProtocolNumber protocol,
                                    PacketBuffer pkt) = 0;
};

// ---------------------------------------------------------------------------
// LinkEndpoint（references/tcpip/stack/registration.go:344-403）
// ---------------------------------------------------------------------------

/**
 * @brief 数据链路层 endpoint（以太网 / loopback / channel / TUN 等）。
 *
 * 网络层通过 WritePacket 经此接口发包；入站路径在 Attach 后调用
 * NetworkDispatcher::DeliverNetworkPacket。
 */
class LinkEndpoint {
 public:
  virtual ~LinkEndpoint() = default;

  /**
   * @brief 最大传输单元（字节），通常含 IP 包上限；无物理网时多为 64KiB 量级。
   */
  virtual uint32_t MTU() const = 0;

  /** @brief 本 endpoint 支持的能力位集。 */
  virtual LinkEndpointCapabilities Capabilities() const = 0;

  /**
   * @brief 链路（及更底层）头部最大长度，供上层在 PacketBuffer::Header
   * 前预留空间。
   */
  virtual uint16_t MaxHeaderLength() const = 0;

  /** @brief 本端链路地址（通常为 6 字节 MAC）。 */
  virtual LinkAddress LinkAddr() const = 0;

  /**
   * @brief 经指定路由发送一个网络层包。
   *
   * 若存在链路头，实现方应设置 pkt.LinkHeader()。
   * pkt.NetworkHeader() / TransportHeader() 在调用前应由上层填好。
   *
   * @param route 出站路由（透明桥接时可用 LocalLinkAddress 等字段）。
   * @param gso 分段卸载参数；无 GSO 时传 nullptr。
   * @param protocol 网络协议号。
   * @param pkt 待发送包（所有权转移给 endpoint）。
   * @return 成功 std::nullopt；失败返回 StackError。
   */
  virtual StackResult WritePacket(Route* route, const GSO* gso,
                                  NetworkProtocolNumber protocol,
                                  PacketBuffer pkt) = 0;

  /**
   * @brief 批量发送（软件 GSO 等）；默认返回 kNotSupported。
   *
   * 对标 WritePackets(r, gso, hdrs, payload, protocol)。
   */
  virtual StackResult WritePackets(Route* route, const GSO* gso,
                                   std::vector<PacketDescriptor> hdrs,
                                   std::vector<uint8_t> payload,
                                   NetworkProtocolNumber protocol) {
    (void)route;
    (void)gso;
    (void)hdrs;
    (void)payload;
    (void)protocol;
    return StackError{ErrorCode::kNotSupported};
  }

  /**
   * @brief 直接发送已含以太网头的原始帧。
   *
   * @param frame 完整链路帧（含以太网头）。
   */
  virtual StackResult WriteRawPacket(std::vector<uint8_t> frame) {
    (void)frame;
    return StackError{ErrorCode::kNotSupported};
  }

  /** @brief 绑定网络层分发器，用于上报入站包。 */
  virtual void Attach(NetworkDispatcher* dispatcher) = 0;

  /** @brief 是否已 Attach。 */
  virtual bool IsAttached() const = 0;

  /**
   * @brief 等待 endpoint 自有工作线程结束（fdbased/tun 等）。
   *
   * loopback 等无异步线程的实现可为空操作。
   */
  virtual void Wait() {}

 protected:
  LinkEndpoint() = default;
};

/**
 * @brief 可注入入站包的 link endpoint（对标 InjectableLinkEndpoint）。
 *
 * 用于 channel 测试：测试代码调用 InjectInbound 模拟收包。
 */
class InjectableLinkEndpoint : public LinkEndpoint {
 public:
  virtual void InjectInbound(NetworkProtocolNumber protocol,
                             PacketBuffer pkt) = 0;

  /**
   * @brief 注入完整出站帧（dest 用于多目标 raw endpoint）。
   */
  virtual StackResult InjectOutbound(Address dest,
                                     std::vector<uint8_t> packet) {
    (void)dest;
    (void)packet;
    return StackError{ErrorCode::kNotSupported};
  }
};

}  // namespace netstack::stack
