/**
 * @file checksum.hh
 * @brief Internet 校验和（RFC 1071），供 IPv4/UDP/TCP 等头部使用。
 *
 * 本文件属于 **header 模块**（无状态、不持有报文缓冲区所有权）。
 * 对标 references/tcpip/header/checksum.go。
 *
 * IPv4 头校验和只覆盖 IP 头本身；传输层还会在「伪首部」上再算一次（后续
 * milestone）。
 *
 * @see RFC 1071
 * @see netstack::header::IPv4Header::CalculateChecksum
 */

#pragma once

#include <cstdint>
#include <span>

namespace netstack::header {

/**
 * @brief 对字节序列计算 Internet 校验和（16 位 one's complement）。
 *
 * 算法要点（教学）：
 * - 按 16 位大端字累加到 32 位 sum；
 * - 奇数字节时最后一个字节放在高 8 位；
 * - 将 sum 的高 16 位与低 16 位反复相加直到无进位；
 * - 返回值**未取反**；写入 IP 头时通常存 `~Checksum(...)`。
 *
 * @param data 参与计算的字节（如整个 IPv4 头，且校验和字段应先置 0）。
 * @param initial 累加初值，用于分段组合（见 ChecksumCombine）。
 * @return 折叠后的 16 位和（非反码形式）。
 */
uint16_t Checksum(std::span<const uint8_t> data, uint16_t initial = 0);

/**
 * @brief 合并两段已折叠的 16 位部分和（含进位回卷）。
 *
 * 在更新 IP 头 Total Length / Checksum
 * 等字段时，可避免重算整个头（EncodePartial）。
 *
 * @param a 第一部分和。
 * @param b 第二部分和。
 * @return 合并并折叠后的 16 位值。
 */
uint16_t ChecksumCombine(uint16_t a, uint16_t b);

}  // namespace netstack::header
