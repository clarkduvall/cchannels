all:
	gcc -Wall main.c chan.c chan.h -pthread -o chan -O3
