CC = gcc
CFLAGS = -std=c11 -O2 -pthread
TARGETS = factorial server client
LIBRARY = multmodulo

all: $(TARGETS)

# Библиотека
$(LIBRARY).o: $(LIBRARY).c $(LIBRARY).h
	$(CC) $(CFLAGS) -c $<

# Программы
factorial: factorial.c
	$(CC) $(CFLAGS) -o $@ $<

server: server.c $(LIBRARY).o
	$(CC) $(CFLAGS) -o $@ $^

client: client.c $(LIBRARY).o
	$(CC) $(CFLAGS) -o $@ $^

# Очистка
clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean