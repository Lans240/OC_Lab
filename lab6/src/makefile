CC = gcc
CFLAGS = -std=c11 -pthread
TARGETS = factorial server client

all: $(TARGETS)

factorial: factorial.c
	$(CC) $(CFLAGS) -o $@ $<

server: server.c
	$(CC) $(CFLAGS) -o $@ $<

client: client.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean