CC = gcc
CFLAGS = -Wall -g
.PHONY: clean

all: hw clean

hw: threads.o hw.o
	$(CC) $(CFLAGS) hw.o threads.o -o hw

hw.o: hw.c
	$(CC) $(CFLAGS) -c hw.c

threads.o: threads.c
	$(CC) $(CFLAGS) -c threads.c	

clean:
	rm -rf *.o *.dSYM
	ipcrm -a