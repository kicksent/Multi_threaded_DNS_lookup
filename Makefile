CC = gcc
CFLAGS = -c -g -Wall -Wextra
LFLAGS = -Wall -Wextra -pthread

.PHONY: all clean

all: multilookup


multilookup: multilookup.o queue.o util.o
	$(CC) $(LFLAGS) $^ -o $@

lookup.o: lookup.c
	$(CC) $(CFLAGS) $<

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) $<

util.o: util.c util.h
	$(CC) $(CFLAGS) $<

multilookup.o: multilookup.c 
	$(CC) $(CFLAGS) $<

clean:
	rm -f lookup queueTest pthread-hello
	rm -f *.o
	rm -f *~
	rm -f results.txt
	rm -f multi_results.txt
