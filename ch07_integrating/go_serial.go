package main

import (
	"fmt"
	"log"
	"os"
	"time"

	"github.com/jacobsa/go-serial/serial"
)

func main() {

	// Set up options.
	options := serial.OpenOptions{
		PortName:        os.Getenv("ARDUINO_PORT"),
		BaudRate:        115200,
		DataBits:        8,
		StopBits:        1,
		MinimumReadSize: 4,
	}

	// Open the port.
	port, err := serial.Open(options)
	if err != nil {
		log.Fatalf("serial.Open: %v", err)
	}

	// it takes a short while for magic to happen (on arduino?)
	time.Sleep(time.Second * 2)

	// Make sure to close it later.
	defer port.Close()

	b := []byte(os.Args[1])
	n, err := port.Write(b)
	if err != nil {
		log.Fatalf("port.Write: %v", err)
	}

	fmt.Println("Wrote", n, "bytes: ", b)
}
