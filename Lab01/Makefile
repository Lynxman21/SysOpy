CC = gcc
CFLAGS = -Wall -std=c17 -g

all: countdown

countdown: countdown.c
	$(CC) $(CFLAGS) -o countdown countdown.c

.PHONY: all clean
clean: 
	rm -f countdown