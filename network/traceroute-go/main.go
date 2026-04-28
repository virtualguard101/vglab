package main

import (
	"errors"
	"fmt"
	"net"
	"os"
	"syscall"
	"time"
)

const (
	TracerouteMaxTTL   = 25
	TracerouteBasePort = 33434
	ProbeAttemptCount  = 3
	ReadTimeout        = 1200 * time.Millisecond
)

func traceroute(targetIP net.IP) error {
	recvConn, err := net.ListenPacket("ip4:icmp", "0.0.0.0")
	if err != nil {
		return err
	}
	defer recvConn.Close()

	seenDestination := false
	for ttl := 1; ttl <= TracerouteMaxTTL; ttl++ {
		destPort := TracerouteBasePort + ttl - 1
		dstAddr := &net.UDPAddr{IP: targetIP, Port: destPort}

		sendConn, err := net.DialUDP("udp4", nil, dstAddr)
		if err != nil {
			return err
		}

		if err := setSocketTTL(sendConn, ttl); err != nil {
			sendConn.Close()
			return err
		}

		routers := make([]string, 0, ProbeAttemptCount)
		routerSet := make(map[string]struct{})

		for attempt := 0; attempt < ProbeAttemptCount; attempt++ {
			_, _ = sendConn.Write([]byte("traceroute probe"))

			deadline := time.Now().Add(ReadTimeout)
			ip, ok := probeResponse(recvConn, destPort, deadline)
			if !ok || ip == nil {
				continue
			}

			hop := ip.String()
			if _, exists := routerSet[hop]; !exists {
				routerSet[hop] = struct{}{}
				routers = append(routers, hop)
			}
			if ip.Equal(targetIP) {
				seenDestination = true
			}
		}

		_ = sendConn.Close()
		printTTLResult(ttl, routers)

		if seenDestination {
			break
		}
	}

	return nil
}

func main() {
	args := os.Args[1:]
	if len(args) == 0 {
		fmt.Fprintf(os.Stderr, "usage: %s <host>\n", os.Args[0])
		os.Exit(1)
	}

	ip_addr := GetHostByName(args[0])
	fmt.Printf("traceroute to %s (%s)\n", args[0], ip_addr.String())

	if err := traceroute(ip_addr); err != nil {
		if errors.Is(err, syscall.EPERM) || errors.Is(err, os.ErrPermission) {
			fmt.Fprintf(os.Stderr, "traceroute failed: %v (try running with sudo or CAP_NET_RAW)\n", err)
			os.Exit(1)
		}
		fmt.Fprintf(os.Stderr, "traceroute failed: %v\n", err)
		os.Exit(1)
	}
}
