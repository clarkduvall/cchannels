all:
	gcc -Wall main.c go.c go.h -pthread -o cgo -g
