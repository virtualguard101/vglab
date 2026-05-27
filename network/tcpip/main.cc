#include <cstdint>
#include <iostream>

#include "ip.hh"
#include "tcp.hh"
#include "utils.hh"

int main() {
  // Build a minimal TCP segment (no options, small payload)
  TCPPacket tcp{};
  tcp.header.src_port = 12345;
  tcp.header.dst_port = 80;
  tcp.header.seq_num = 1;
  tcp.header.ack_num = 0;
  tcp.header.data_offset = 20;  // bytes
  tcp.header.reserved = 0;
  tcp.header.flags = TCPFlags{
      .fin = false,
      .syn = true,
      .rst = false,
      .psh = false,
      .ack = false,
      .urg = false,
      .ece = false,
      .cwr = false,
  };
  tcp.header.window_size = 65535;
  tcp.header.checksum = 0;  // filled after encoding (needs pseudo-header)
  tcp.header.urgent_pointer = 0;
  tcp.header.options = {};
  tcp.payload = ByteVector{
      0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x2C, 0x20,
      0x77, 0x6F, 0x72, 0x6C, 0x64, 0x21,
  };  // "Hello, world!"

  auto tcp_bytes_opt = Encoder(tcp);
  if (!tcp_bytes_opt) {
    std::cerr << "TCP encode failed\n";
    return 1;
  }
  ByteVector tcp_bytes = *tcp_bytes_opt;

  // IPv4 endpoints for pseudo-header
  IPv4Address src = *IPv4Address::Parse("10.0.0.1");
  IPv4Address dst = *IPv4Address::Parse("10.0.0.2");

  // Write TCP checksum into bytes (offset 16)
  WriteBE16(tcp_bytes.data() + 16, 0);
  const auto tcp_cs = CalculateTCPChecksum(
      tcp_bytes.data(), tcp_bytes.size(), src.octets.data(), dst.octets.data());
  if (!tcp_cs) {
    std::cerr << "TCP checksum calc failed\n";
    return 1;
  }
  WriteBE16(tcp_bytes.data() + 16, *tcp_cs);

  if (!VerifyTCPChecksum(tcp_bytes.data(), tcp_bytes.size(), src.octets.data(),
                         dst.octets.data())) {
    std::cerr << "TCP checksum verify failed\n";
    return 1;
  }

  // Wrap in IPv4
  IPv4Packet ip{};
  ip.header.version = 4;
  ip.header.header_len = 20;
  ip.header.type_of_service = 0;
  ip.header.total_length = static_cast<uint16_t>(20 + tcp_bytes.size());
  ip.header.identification = 0;
  ip.header.flags = 0;
  ip.header.fragment_offset = 0;
  ip.header.time_to_live = 64;
  ip.header.protocol = 6;         // TCP
  ip.header.header_checksum = 0;  // computed by IPv4 Encoder
  ip.header.src_addr = src;
  ip.header.dst_addr = dst;
  ip.header.options = {};
  ip.payload = tcp_bytes;

  auto ip_bytes_opt = Encoder(ip);
  if (!ip_bytes_opt) {
    std::cerr << "IPv4 encode failed\n";
    return 1;
  }
  const ByteVector ip_bytes = *ip_bytes_opt;

  // Decode IPv4
  auto ip_dec = Decoder(ip_bytes.data());
  if (!ip_dec) {
    std::cerr << "IPv4 decode failed\n";
    return 1;
  }

  // Decode TCP (with checksum verification via pseudo-header)
  auto tcp_dec = Decoder(ip_dec->payload.data(), ip_dec->payload.size(),
                         ip_dec->header.src_addr.octets.data(),
                         ip_dec->header.dst_addr.octets.data());
  if (!tcp_dec) {
    std::cerr << "TCP decode failed\n";
    return 1;
  }

  std::cout << "OK: IPv4 " << ip_dec->header.src_addr.ToString() << " -> "
            << ip_dec->header.dst_addr.ToString() << ", TCP "
            << tcp_dec->header.src_port << " -> " << tcp_dec->header.dst_port
            << ", payload_len=" << tcp_dec->payload.size() << "\n";

  return 0;
}
