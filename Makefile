CC=gcc
CFLAGS=-Wall -g

.PHONY: all clean

all: words

words: words.o
	$(CC) $(CFLAGS) -o words words.o

words.o: words.c
	$(CC) $(CFLAGS) -c words.c

clean:
	rm -f words words.o

