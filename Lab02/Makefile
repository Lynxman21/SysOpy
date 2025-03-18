CC = gcc
CFLAGS = -Wall -std=c17 -g

.PHONY: all clean

all: libcollatz.a libcollatz.so client_static client_shared client_dynamic

libcollatz.a: collatz.c 
	$(CC) -c collatz.c 
	ar rcs libcollatz.a collatz.o

libcollatz.so: collatz.c 
	$(CC) -fPIC -c collatz.c
	$(CC) -shared collatz.o -o libcollatz.so

client_static: libcollatz.a client.c
	$(CC) $(CFLAGS) client.c libcollatz.a -o client_static

client_shared: client.c libcollatz.so
	$(CC) $(CFLAGS) client.c -lcollatz -L./ -Wl,-rpath,. -o client_shared

client_dynamic: client.c libcollatz.so
	$(CC) $(CFLAGS) client.c -L./ -lcollatz -Wl,-rpath,. -DD=1 -o client_dynamic 

clean:
	rm -f *.o *.a *.so client_*
