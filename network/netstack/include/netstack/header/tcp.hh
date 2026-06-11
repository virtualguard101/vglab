/**
 * @file tcp.hh
 * @brief TCP 报文头视图（M2 无选项，固定 20 字节）。
 *
 * ## TCP 头布局（RFC 793，M2 最小 20 字节）
 *
 * @code
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |          Source Port          |       Destination Port        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        Sequence Number                        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    Acknowledgment Number                      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Data Offset  |    Reserved   |N|C|E|U|A|P|R|S|F|   Window    |
 * |   (4 bits)    |               |G|W|C|R|C|S|S|Y|I|    Size      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |           Checksum            |         Urgent Pointer        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    Options (M2 无) / Padding                  |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endcode
 *
 * - **Data Offset**：头长度，单位 4 字节；M2 恒为 5（即 20 字节）。
 * - **Flags**（低 6 位常用）：FIN/SYN/RST/PSH/ACK。
 * - **Window Size**：接收窗口；M2 常数 65535。
 * - **Protocol 号**：在 IPv4 头里为 6（`kTCPProtocolNumber`）。
 *
 * ## Flags 与 RFC 793 状态转移（M2 常用组合）
 *
 * | 段类型 | Flags | 典型转移（服务端被动打开） |
 * |--------|-------|---------------------------|
 * | SYN | S | LISTEN → SYN-RECEIVED |
 * | SYN-ACK | S+A | （出站）SYN-RECEIVED 等待 ACK |
 * | ACK | A | SYN-RECEIVED → ESTABLISHED |
 * | 数据 | A+P | ESTABLISHED（保持） |
 * | FIN | F+A | ESTABLISHED → CLOSE-WAIT / 出站 LAST-ACK |
 * | 纯 ACK | A | CLOSE-WAIT / LAST-ACK → CLOSED |
 *
 * @see docs/tcp-rfc793-states.md
 * @see RFC 793 Figure 6
 * @see references/tcpip/header/tcp.go
 * @see docs/m2.md
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace netstack::header {

/** @brief 无选项时 TCP 头最小长度（字节）。 */
inline constexpr size_t kTCPMinimumSize = 20;

/** @brief IP 协议字段中的 TCP 编号。 */
inline constexpr uint8_t kTCPProtocolNumber = 6;

/** @name TCP 头内各字段的字节偏移 */
///@{
inline constexpr size_t kTCPSrcPort = 0;
inline constexpr size_t kTCPDstPort = 2;
inline constexpr size_t kTCPSeqNum = 4;
inline constexpr size_t kTCPAckNum = 8;
/** @brief 第 12 字节高 4 位 = Data Offset（32 位字个数）。 */
inline constexpr size_t kTCPDataOffset = 12;
/** @brief 第 13 字节低 6 位 = 控制标志位。 */
inline constexpr size_t kTCPFlagsOffset = 13;
inline constexpr size_t kTCPWindowSize = 14;
inline constexpr size_t kTCPChecksum = 16;
///@}

/**
 * @brief TCP 控制标志（与 references/tcpip/header/tcp.go 位序一致）。
 *
 * 可组合：`kSyn | kAck` 表示 SYN-ACK 段。
 */
enum class TCPFlags : uint8_t {
  kFin = 0x01,  ///< 结束连接
  kSyn = 0x02,  ///< 同步序列号（握手）
  kRst = 0x04,  ///< 复位连接
  kPsh = 0x08,  ///< 推送数据到应用层
  kAck = 0x10,  ///< 确认号有效
};

constexpr TCPFlags operator|(TCPFlags a, TCPFlags b) {
  return static_cast<TCPFlags>(static_cast<uint8_t>(a) |
                               static_cast<uint8_t>(b));
}

/** @brief 检查 flags 字节是否包含某位。 */
constexpr bool HasFlag(uint8_t flags, TCPFlags f) {
  return (flags & static_cast<uint8_t>(f)) != 0;
}

/**
 * @brief 编码 TCP 头时使用的逻辑字段（对标 TCPFields）。
 *
 * data_offset 为**字节数**（M2 填 20）；Encode 时转换为 32 位字数写入头。
 */
struct TCPFields {
  uint16_t src_port{};
  uint16_t dst_port{};
  uint32_t seq_num{};
  uint32_t ack_num{};
  uint8_t data_offset = kTCPMinimumSize;
  uint8_t flags{};
  uint16_t window_size = 65535;
  uint16_t checksum{};
};

/**
 * @brief TCP 头缓冲区视图（不拥有内存，对标 `type TCP []byte`）。
 *
 * 载荷从 `DataOffset()` 字节处开始；M2 无选项时偏移恒为 20。
 */
class TCPHeader {
 public:
  explicit TCPHeader(std::span<uint8_t> data) : data_(data) {}

  uint16_t SourcePort() const;
  uint16_t DestinationPort() const;
  uint32_t SequenceNumber() const;
  uint32_t AcknowledgmentNumber() const;
  /** @return 头长度（字节）= (data[12] >> 4) * 4。 */
  uint8_t DataOffset() const;
  uint8_t Flags() const;
  uint16_t WindowSize() const;

  void Encode(const TCPFields& fields);

  /** @brief demuxer 用：从仍以 TCP 头开头的 buffer 解析源/目的端口。 */
  static bool ParsePorts(std::span<const uint8_t> data, uint16_t& src,
                         uint16_t& dst);

  /**
   * @brief 结构合法性：缓冲区足够、DataOffset 在 [20, tcp_size] 内。
   */
  bool IsValid(size_t tcp_size) const;

  size_t HeaderLength() const { return DataOffset(); }

  /** @brief TCP 头之后的载荷切片（可能为空）。 */
  std::span<const uint8_t> Payload() const;

 private:
  std::span<uint8_t> data_;
};

}  // namespace netstack::header
