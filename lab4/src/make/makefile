CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2
TARGETS = parallel_min_max process_memory

all: $(TARGETS)

# Сборка parallel_min_max (задание 1)
parallel_min_max: parallel_min_max.o find_min_max.o utils.o
	$(CC) $(CFLAGS) -o $@ $^

parallel_min_max.o: parallel_min_max.c find_min_max.h utils.h
	$(CC) $(CFLAGS) -c $<

find_min_max.o: find_min_max.c find_min_max.h utils.h
	$(CC) $(CFLAGS) -c $<

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c $<

# Сборка process_memory (задание 3) с флагом -w
process_memory: process_memory.o
	$(CC) -w -std=c11 -O2 -o $@ $<

process_memory.o: process_memory.c
	$(CC) -w -std=c11 -O2 -c $<

# Очистка
clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean