package main

import (
	"fmt"
	"os"
)

func main() {
	args := os.Args[1:]
	ip_addr := GetHostByName(args[0])
	fmt.Printf("traceroute to %s (%s)\n", args[0], ip_addr.String())
}
