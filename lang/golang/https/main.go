package main

import (
	"fmt"
	"log"
	"net/http"
)

func hello(w http.ResponseWriter, req *http.Request) {

	fmt.Fprintf(w, "hello\n")
}

func headers(w http.ResponseWriter, req *http.Request) {

	for name, headers := range req.Header {
		for _, h := range headers {
			fmt.Fprintf(w, "%v: %v\n", name, h)
		}
	}
}

func main() {
	log.Println("Starting server on port 8090...")
	err := http.ListenAndServe(":8090", nil)
	if err != nil {
		log.Fatalf("Error starting server: %v", err)
	}

	http.HandleFunc("/hello", hello)
	http.HandleFunc("/headers", headers)
}
