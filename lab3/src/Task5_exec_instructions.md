

## Обновлённый Makefile

``` makefile
# Makefile — сборка sequential_min_max, parallel_min_max и run_sequential
CC := gcc
CFLAGS := -I. -Wall -Wextra -std=c11 -O2

.PHONY: all clean

# Цель по умолчанию — собрать всё
all: sequential_min_max parallel_min_max run_sequential

# Собираем последовательную программу
sequential_min_max: sequential_min_max.c find_min_max.o utils.o
    $(CC) $(CFLAGS) -o $@ sequential_min_max.c find_min_max.o utils.o

# Собираем параллельную программу
parallel_min_max: parallel_min_max.c find_min_max.o utils.o
    $(CC) $(CFLAGS) -o $@ parallel_min_max.c find_min_max.o utils.o

# Собираем программу-обёртку для запуска sequential_min_max
run_sequential: run_sequential.c
    $(CC) $(CFLAGS) -o $@ run_sequential.c

# Объектные файлы
find_min_max.o: find_min_max.c find_min_max.h utils.h
    $(CC) $(CFLAGS) -c -o $@ find_min_max.c

utils.o: utils.c utils.h
    $(CC) $(CFLAGS) -c -o $@ utils.c

# Очистка
clean:
    rm -f utils.o find_min_max.o sequential_min_max parallel_min_max run_sequential
```

