(CC) = gcc
(CFLAGS) =-Wall -std=c17 -g

.PHONY: all clean zad2

all: zad1 sender counter

zad1: zad1.c
	$(CC) $(CFLAGS) -o zad1 zad1.c

sender: sender.c
	$(CC) $(CFLAGS) -o sender sender.c

counter: counter.c
	$(CC) $(CFLAGS) -o counter counter.c

zad2:
	make sender
	make counter

clean:
	rm -f zad1 sender counter