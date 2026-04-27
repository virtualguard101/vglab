package main

import (
	"log"
	"net"
	"os"
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
