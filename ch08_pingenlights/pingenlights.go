package main

import (
	"bytes"
	"errors"
	"fmt"
	"log"
	"os"
	"os/exec"
	"regexp"
	"strconv"
	"strings"
	"time"

	"github.com/jacobsa/go-serial/serial"
)

type Source struct {
	Name     string
	Provider string
	Endpoint string
	Options  map[string]string
}

func getPingTimeFromOutput(out string) (ms int, err error) {
	lines := strings.Split(out, "\n")
	if len(lines) == 0 {
		return 0, errors.New("ping failed")
	}
	lastLine := lines[len(lines)-1]
	if _, err := regexp.MatchString(lastLine, "0 packets received"); err == nil {
		return 0, errors.New("host unavailable")
	}
	stuff := strings.Split(lastLine, "/")
	i, err := strconv.Atoi(stuff[4]) // avg round trip
	if err == nil {
		return 0, errors.New("can't parse ping output")
	}
	return i, nil
}

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

	for {
		cmd := exec.Command("ping", "-c1", os.Args[1])
		var out bytes.Buffer
		cmd.Stdout = &out
		if err != nil {
			log.Fatalf("ping failed: %s", err)
		}

		pingTime, err := getPingTimeFromOutput(out.String())
		var ledData string
		if err != nil {
			ledData = "YFF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000FF0000Z"
			log.Printf("ping did not complete: %s", err)
			time.Sleep(time.Second)
			continue
		} else {
			ledData = "Y"
			for i := 0; i < pingTime; i++ {
				ledData += "00FF00"
			}
			ledData += "Z"
		}

		b := []byte(ledData)
		n, err := port.Write(b)
		if err != nil {
			log.Fatalf("port.Write: %v", err)
		}
		fmt.Println("Wrote", n, "bytes: ", b)
	}

}
