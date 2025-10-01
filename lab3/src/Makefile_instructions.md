# Задание 4 --- Makefile

Ниже --- коротко и по делу: разбор имеющегося `Makefile`, что делает
каждая цель, затем обновлённый `Makefile` с целью `all` и командами
сборки/очистки. В конце --- краткая инструкция по использованию и пара
рекомендаций.

## Разбор существующего Makefile

Содержимое вашего makefile (коротко):

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

### Объяснение целей

-   `CC` --- компилятор (gcc).
-   `CFLAGS` --- флаги компиляции (сейчас только `-I.` для include).
-   `sequential_min_max` --- цель для сборки последовательной программы.
    Зависит от `utils.o` и `find_min_max.o` (и заголовков). Рецепт
    линкует объектные файлы и исходник `sequential_min_max.c` в
    исполняемый файл `sequential_min_max`.
-   `parallel_min_max` --- аналогично для параллельной программы.
-   `utils.o` --- цель сборки объектного файла `utils.o` из `utils.c`. В
    рецепте флаг `-c` создаёт объектный файл.
-   `find_min_max.o` --- цель сборки `find_min_max.o`.
-   `clean` --- удаляет сгенерированные файлы.

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

## Как пользоваться

В каталоге с исходниками (там, где `Makefile` и `.c/.h` файлы):

1.  Построить всё:

``` bash
make
# или
make all
```

2.  Построить только одну программу:

``` bash
make sequential_min_max
# или
make parallel_min_max
```

3.  Очистить:

``` bash
make clean
```

## Проверка работоспособности

После `make` проверьте запуск:

``` bash
# последовательная:
./sequential_min_max 123 1000

# параллельная (через pipe):
./parallel_min_max --seed 123 --array_size 1000000 --pnum 4

# параллельная (через файлы):
./parallel_min_max --seed 123 --array_size 1000000 --pnum 4 --by_files
```

## Рекомендации

-   Если при компиляции появится предупреждение об «unused variable
    `current_optind`», удалите объявление `int current_optind = ...` или
    добавьте `(void)current_optind;`.
-   Можно расширить makefile, добавив цели `run-seq`/`run-par` для
    удобного тестирования.
-   Можно добавить цель `test`, которая соберёт и запустит оба бинарника
    и сравнит результаты.
