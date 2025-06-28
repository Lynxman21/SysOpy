CC = gcc
CFLAGS = -Wall -std=c17 -g

.PHONY: all clean
all: main

main: main.c
	$(CC) $(CFLAGS) -o main main.c -lpthread -lrt

clean:
	rm -f main