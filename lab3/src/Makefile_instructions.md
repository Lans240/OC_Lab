

``` makefile
CC=gcc
CFLAGS=-I.

sequential_min_max : utils.o find_min_max.o utils.h find_min_max.h
    $(CC) -o sequential_min_max find_min_max.o utils.o sequential_min_max.c $(CFLAGS)

parallel_min_max : utils.o find_min_max.o utils.h find_min_max.h
    $(CC) -o parallel_min_max utils.o find_min_max.o parallel_min_max.c $(CFLAGS)

utils.o : utils.h
    $(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o : utils.h find_min_max.h
    $(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

clean :
    rm utils.o find_min_max.o sequential_min_max parallel_min_max
```


## Обновлённый Makefile (с целью `all`)

``` makefile
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
```
