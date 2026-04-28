package main

import (
	"errors"
	"fmt"
	"log"
	"net"
	"os"
	"strings"
	"syscall"
	"time"
)

func GetHostByName(host string) net.IP {
	ipAddr, err := net.LookupIP(host)
	if err != nil {
		log.Fatalf("Error looking up IP address: %v", err)
		panic(err)
	}
	if len(ipAddr) == 0 {
		log.Fatalf("No IP address found for host: %s", host)
		os.Exit(1)
	}
	return ipAddr[0]
}

func CheckTTLExpired(icmp ICMP) bool {
	return icmp.Type == 11 && icmp.Code == 0
}

func CheckUnreachable(icmp ICMP) bool {
	return icmp.Type == 3 && icmp.Code == 3
}

func setSocketTTL(conn *net.UDPConn, ttl int) error {
	rawConn, err := conn.SyscallConn()
	if err != nil {
		return err
	}

	var sockErr error
	if err := rawConn.Control(func(fd uintptr) {
		sockErr = syscall.SetsockoptInt(int(fd), syscall.IPPROTO_IP, syscall.IP_TTL, ttl)
	}); err != nil {
		return err
	}
	return sockErr
}

func probeResponse(recvsock net.PacketConn, expectedDstPort int, deadline time.Time) (net.IP, bool) {
	buf := make([]byte, 1500)

	for {
		if err := recvsock.SetReadDeadline(deadline); err != nil {
			return nil, false
		}

		n, addr, err := recvsock.ReadFrom(buf)
		if err != nil {
			var netErr net.Error
			if errors.As(err, &netErr) && netErr.Timeout() {
				return nil, false
			}
			continue
		}
		if n == 0 {
			continue
		}

		packet := buf[:n]
		icmpStart := 0

		// On Linux, ip4:icmp sockets may return either:
		// 1) full IPv4 packet (outer IP header + ICMP), or
		// 2) ICMP payload directly (without outer IP header).
		// Support both forms to avoid dropping all valid replies.
		if len(packet) >= 20 && int(packet[0]>>4) == 4 {
			outerIPv4 := ParseIPv4(packet)
			outerHeaderBytes := outerIPv4.HeaderLen * 4
			if outerHeaderBytes < 20 || len(packet) < outerHeaderBytes+8 {
				continue
			}
			if outerIPv4.Protocol != 1 {
				continue
			}
			icmpStart = outerHeaderBytes
		}

		if len(packet) < icmpStart+8 {
			continue
		}

		icmp := ParseICMP(packet[icmpStart:])
		if !(CheckTTLExpired(icmp) || CheckUnreachable(icmp)) {
			continue
		}

		quotedStart := icmpStart + 8
		if len(packet) < quotedStart+20 {
			continue
		}

		quotedIPv4 := ParseIPv4(packet[quotedStart:])
		quotedHeaderBytes := quotedIPv4.HeaderLen * 4
		udpStart := quotedStart + quotedHeaderBytes
		if quotedHeaderBytes < 20 || len(packet) < udpStart+8 {
			continue
		}

		udp := ParseUDP(packet[udpStart:])
		if udp.DstPort != expectedDstPort {
			continue
		}

		if ipAddr, ok := addr.(*net.IPAddr); ok {
			return ipAddr.IP, true
		}
		return net.ParseIP(addr.String()), true
	}
}

func printTTLResult(ttl int, routers []string) {
	if len(routers) == 0 {
		fmt.Printf("%2d  *\n", ttl)
		return
	}
	fmt.Printf("%2d  %s\n", ttl, strings.Join(routers, " "))
}
