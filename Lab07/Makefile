(CC) = gcc
(CFLAGS) = -Wall -std=c17 -g -Wextra

.PHONY: all clean

all: server client

client: client.c
	$(CC) $(CFLAGS) -o client client.c

server: server.c
	$(CC) $(CFLAGS) -o server server.c

clean:
	rm -f server client