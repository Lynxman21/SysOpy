CC = gcc
CFLAGS = -Wall -lpthread

.PHONY: all clean

all:
	make client
	make server

client: client.c
	$(CC) client.c $(CFLAGS) -o client

server: server.c
	$(CC) server.c $(CFLAGS) -o server

clean:
	rm client server