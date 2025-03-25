(CC)=gcc
(CFLAGS)=-Wall -std=c17 -g

.PHONY: all clean run

all: flipper

flipper: flipper.c
	$(CC) $(CFLAGS) -o flipper flipper.c

run:
	make flipper
	./flipper ./art ./output

clean: 
	rm flipper ./output/*
	rmdir ./output