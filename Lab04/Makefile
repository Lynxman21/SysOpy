(CC)=gcc
(CFLAGS)=-Wall -std=c17 -g

.PHONY: clean all run2

all: zad1 run2

zad1: zad1.c
	$(CC) $(CFLAGS) -o zad1 zad1.c

zad2: zad2.c
	$(CC) $(CFLAGS) -o zad2 zad2.c

run2:
	make zad2
	./zad2 /home/lynxman/SysOpy

clean:
	rm -f zad1 zad2