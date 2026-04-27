package main

import "net"

type IPv4 struct {
	Version        int    // 4 bits, Version of the IP protocol being used.
	HeaderLen      int    // 4 bits, Length of the IP header. Measured in 4-byte words.
	TOS            int    // 8 bits, Type of Service.
	Length         int    // 16 bits, Total length of the packet.
	ID             int    // 16 bits, Identification number.
	Flags          int    // 3 bits, Flags. 0 for no flags, 1 for more fragments, 2 for don't fragment, 3 for don't fragment and more fragments.
	FragOffset     int    // 13 bits, Fragment offset.
	TTL            int    // 8 bits, Time to live.
	Protocol       int    // 8 bits, Protocol. 1 for ICMP, 6 for TCP, 17 for UDP.
	HeaderChecksum int    // 16 bits, Header checksum.
	SourceIP       net.IP // 32 bits, Source IP address.
	DestinationIP  net.IP // 32 bits, Destination IP address.
}

type ICMP struct {
	Type     int    // 1 byte, Type of the ICMP message. 1 for echo request, 0 for echo reply.
	Code     int    // 1 byte, Code of the ICMP message. 0 for no code.
	Checksum int    // 2 bytes, Checksum of the ICMP message.
	Data     []byte // Data of the ICMP message.
}

type UDP struct {
	SrcPort  int    // 16 bits, Source port.
	DstPort  int    // 16 bits, Destination port.
	Length   int    // 16 bits, Length of the UDP header.
	Checksum int    // 16 bits, Checksum of the UDP header.
	Data     []byte // Data of the UDP message.
}
