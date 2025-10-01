# Makefile — сборка sequential_min_max и parallel_min_max
CC := gcc
CFLAGS := -I. -Wall -Wextra -std=c11 -O2

.PHONY: all clean

all: sequential_min_max parallel_min_max

# Собираем последовательную программу
sequential_min_max: sequential_min_max.c find_min_max.o utils.o
	$(CC) $(CFLAGS) -o $@ sequential_min_max.c find_min_max.o utils.o

# Собираем параллельную программу
parallel_min_max: parallel_min_max.c find_min_max.o utils.o
	$(CC) $(CFLAGS) -o $@ parallel_min_max.c find_min_max.o utils.o

# Объектные файлы
find_min_max.o: find_min_max.c find_min_max.h utils.h
	$(CC) $(CFLAGS) -c -o $@ find_min_max.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c -o $@ utils.c

# Удаление результатов сборки
clean:
	rm -f utils.o find_min_max.o sequential_min_max parallel_min_max
