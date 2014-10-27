package main

import (
	"fmt"
	"runtime"
)

func grab_from_chan(c chan int, end chan int) {
	var acc int;
	for v := range c {
		acc += v
	}
	end <- acc
}

func add_to_chan(c chan int, end chan int) {
	for i := 0; i < 10000000; i++ {
		c <- i
	}
	close(c)
	end <- 0
}

func main() {
	numThreads := 5
	runtime.GOMAXPROCS(numThreads + 1)
	c := make(chan int, 10)
	end := make(chan int, 1)
	go add_to_chan(c, end)

	for i := 0; i < numThreads; i++ {
		go grab_from_chan(c, end)
	}

	var acc int;
	for i := 0; i < numThreads + 1; i++ {
		v := <-end
		acc += v;
	}

	fmt.Println("Total Numbers:", acc)
}
