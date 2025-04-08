(CC)=gcc
(CFLAGS)=-Wall -std=c17 -g

.PHONY: all clean

all: zad1 catcher sender

zad1: zad1.c
	$(CC) $(CFLAGS) -o zad1 zad1.c

catcher: catcher.c
	$(CC) $(CFLAGS) -o catcher catcher.c

sender: sender.c
	$(CC) $(CFLAGS) -o sender sender.c

clean:
	rm -f zad1 catcher sender