CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -pthread

all: parallel_sum

# Создание статической библиотеки libsum.a
libsum.a: sum.o
	ar rcs libsum.a sum.o

# Компиляция объектного файла для библиотеки
sum.o: sum.c sum.h
	$(CC) $(CFLAGS) -c sum.c

# Основная программа
parallel_sum: parallel_sum.c utils.c libsum.a
	$(CC) $(CFLAGS) -o parallel_sum parallel_sum.c utils.c -L. -lsum

clean:
	rm -f parallel_sum *.o *.a

.PHONY: all clean