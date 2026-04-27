package main

import (
	"bufio"
	"fmt"
	"log"
	"net/http"
)

func main() {

	// Http get request to the server
	resp, err := http.Get("http://localhost:8090/hello")
	if err != nil {
		log.Fatalf("Error getting response: %v", err)
	}
	// Close the response body after the function ends (use `defer` to delay the execution)
	defer resp.Body.Close()

	fmt.Println("Response status:", resp.Status)

	// Scan the response body
	scanner := bufio.NewScanner(resp.Body)
	for i := 0; scanner.Scan() && i < 5; i++ {
		fmt.Println(scanner.Text())
	}

	if err := scanner.Err(); err != nil {
		log.Fatalf("Error scanning response: %v", err)
	}
}
