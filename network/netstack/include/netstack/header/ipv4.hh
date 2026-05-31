/**
 * @file ipv4.hh
 * @brief IPv4 报文头的常量、逻辑字段与缓冲区视图（编解码）。
 *
 * 本文件属于 **header 模块**（对标 references/tcpip/header/ipv4.go）。
 * 设计要点：
 * - **IPv4Header** 不拥有内存，仅通过 std::span 包装已有缓冲区（类似 Go 的
 * `type IPv4 []byte`）。
 * - 调用除 IPVersion 外的成员前，应先保证缓冲区长度足够，并用 IsValid()
 * 做结构校验。
 *
 * IPv4 头布局（RFC 791，固定前 20 字节，可选选项至最多 60 字节）：
 * @code
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |Version|  IHL  |Type of Service|          Total Length         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |         Identification        |Flags|    Fragment Offset      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Time to Live |    Protocol   |         Header Checksum       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                       Source Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    Destination Address                        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endcode
 *
 * @see docs/m0.md
 * @see RFC 791
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "netstack/address.hh"

namespace netstack::header {

/** @name 字段在 IPv4 头内的字节偏移（从 0 起） */
///@{
/** @brief 第 0 字节：高 4 位 Version，低 4 位 IHL（头长，单位 4 字节）。 */
inline constexpr size_t kVersIHL = 0;
/** @brief 服务类型 / DSCP（本实现仅透传单字节）。 */
inline constexpr size_t kTOS = 1;
/** @brief 整个 IP 报文长度（头 + 载荷），大端 uint16。 */
inline constexpr size_t kTotalLengthOffset = 2;
/** @brief 分片标识，同一 datagram 各片相同。 */
inline constexpr size_t kID = 4;
/** @brief 3 位 Flags + 13 位 Fragment Offset（偏移单位为 8 字节）。 */
inline constexpr size_t kFlagsFO = 6;
inline constexpr size_t kTTL = 8;
/** @brief 上层协议号，如 6=TCP、17=UDP。 */
inline constexpr size_t kProtocol = 9;
inline constexpr size_t kChecksumOffset = 10;
inline constexpr size_t kSrcAddr = 12;
inline constexpr size_t kDstAddr = 16;
///@}

/** @name 协议常量 */
///@{
/** @brief 无选项时 IPv4 头最小长度（字节）。 */
inline constexpr size_t kIPv4MinimumSize = 20;
/** @brief IHL 最大 15 → 15×4 = 60 字节。 */
inline constexpr size_t kIPv4MaximumHeaderSize = 60;
/** @brief 分片时首片载荷至少 8 字节（RFC 791）。 */
inline constexpr size_t kMinIPFragmentPayloadSize = 8;
inline constexpr size_t kIPv4AddressSize = 4;
/** @brief 以太网类型字段中的 IPv4 值（also NetworkProtocolNumber）。 */
inline constexpr uint16_t kIPv4ProtocolNumber = 0x0800;
inline constexpr uint8_t kIPv4Version = 4;
/** @brief 任意 IPv4 实现必须能处理/重组的最小报文长度。 */
inline constexpr size_t kIPv4MinimumProcessableDatagramSize = 576;
///@}

/**
 * @brief IPv4 头中 3 位 Flags 的命名常量（与 13 位 Fragment Offset 共用 16
 * 位字）。
 *
 * 存储时位于 flags+offset 字的高 3 位；SetFlagsFragmentOffset 负责编码。
 */
enum class IPv4Flags : uint8_t {
  MoreFragments = 0x01,  ///< MF：后面还有片
  DontFragment = 0x02,   ///< DF：禁止中间设备分片
};

/** @brief 组合分片标志（用于 Encode 或检测）。 */
constexpr IPv4Flags operator|(IPv4Flags a, IPv4Flags b) {
  return static_cast<IPv4Flags>(static_cast<uint8_t>(a) |
                                static_cast<uint8_t>(b));
}

constexpr IPv4Flags operator&(IPv4Flags a, IPv4Flags b) {
  return static_cast<IPv4Flags>(static_cast<uint8_t>(a) &
                                static_cast<uint8_t>(b));
}

/**
 * @brief 编码 IPv4 头时使用的逻辑字段集合。
 *
 * 与线上布局一一对应，便于测试构造包；Encode() 会写入 data_ 并处理字节序。
 * 地址使用 IPv4Address，而非裸 uint32，与 netstack 核心类型一致。
 */
struct IPv4Fields {
  uint8_t ihl = kIPv4MinimumSize;  ///< 头长度（字节），通常为 20
  uint8_t tos = 0;
  uint16_t total_length = 0;
  uint16_t id = 0;
  uint8_t flags = 0;             ///< 原始 3 位 flags，可用 IPv4Flags 转 uint8_t
  uint16_t fragment_offset = 0;  ///< 字节偏移（内部编码时会 >>3）
  uint8_t ttl = 0;
  uint8_t protocol = 0;
  uint16_t checksum = 0;
  IPv4Address src_addr{};
  IPv4Address dst_addr{};
};

/**
 * @brief 非拥有的 IPv4 头视图。
 *
 * @warning 不检查下标；缓冲区过短时行为未定义。生产路径应先
 * IsValid(packet_size)。
 *
 * 典型用法：
 * @code
 * std::vector<uint8_t> buf(20);
 * IPv4Header hdr(buf);
 * hdr.Encode(fields);
 * hdr.SetChecksumField(~hdr.CalculateChecksum());
 * @endcode
 */
class IPv4Header {
 public:
  /**
   * @param data 至少覆盖整个 IP 头的可写缓冲区；span
   * 生命周期须覆盖本对象使用期。
   */
  explicit IPv4Header(std::span<uint8_t> data) : data_(data) {}

  /**
   * @brief 读取首字节高 4 位 IP 版本。
   * @return 4 表示 IPv4；缓冲区不足返回 -1。
   */
  static int IPVersion(std::span<const uint8_t> data);

  /** @brief 当前 span 长度（未必等于 TotalLength）。 */
  size_t size() const { return data_.size(); }

  /** @brief IHL 字段 ×4，即头长度（字节）。 */
  uint8_t HeaderLength() const;
  uint16_t ID() const;
  uint8_t Protocol() const;
  /** @brief 3 位 flags（已从高 13 位中剥离）。 */
  uint8_t Flags() const;
  uint8_t TTL() const;
  /** @brief 分片偏移（字节，已左移 3 位还原）。 */
  uint16_t FragmentOffset() const;
  uint16_t TotalLength() const;
  uint16_t ChecksumField() const;

  IPv4Address SourceAddress() const;
  IPv4Address DestinationAddress() const;

  /** @brief TotalLength - HeaderLength。 */
  uint16_t PayloadLength() const;
  /** @brief 指向头之后载荷的可写子 span。 */
  std::span<uint8_t> Payload();

  void SetTotalLength(uint16_t total_length);
  void SetChecksumField(uint16_t checksum);
  void SetFlagsFragmentOffset(uint8_t flags, uint16_t fragment_offset);
  void SetID(uint16_t id);
  void SetSourceAddress(IPv4Address addr);
  void SetDestinationAddress(IPv4Address addr);

  /**
   * @brief 计算头校验和（校验和字段应已为 0）。
   * @return 未取反的 16 位和；写入头时用 `~CalculateChecksum()`。
   */
  uint16_t CalculateChecksum() const;

  /**
   * @brief 验证头校验和：对整个头算和后应为 0xFFFF（ones' complement 语义）。
   *
   * 参考实现接收路径未强制此检查；本教学栈在 M0 显式校验（见 docs/m0.md）。
   */
  bool IsChecksumValid() const;

  /**
   * @brief 结构合法性：版本、IHL、TotalLength 与 packet_size 关系。
   * @param packet_size 底层缓冲区或链路层提供的总长度上界。
   * @note 不验证校验和；需结合 IsChecksumValid()。
   */
  bool IsValid(size_t packet_size) const;

  /** @brief 按 fields 写入各字段（不自动填校验和）。 */
  void Encode(const IPv4Fields& fields);

  /**
   * @brief 仅更新 Total Length 与 Checksum（相似包批量发送时优化）。
   * @param partial_checksum 不含 TotalLength/Checksum 两字段时的部分和。
   */
  void EncodePartial(uint16_t partial_checksum, uint16_t total_length);

 private:
  std::span<uint8_t> data_;
};

/**
 * @brief 是否为 IPv4 组播地址（224.0.0.0/4，首字节高 4 位为 0xE）。
 * @see references/header.IsV4MulticastAddress
 */
bool IsV4MulticastAddress(IPv4Address addr);

}  // namespace netstack::header
