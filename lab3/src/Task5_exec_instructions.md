# Задание 5 --- Использование exec и Makefile

Я отвечаю как всемирно известный преподаватель по системному
программированию.

В этом задании нужно написать программу, которая запускает ваше
приложение `sequential_min_max` в отдельном процессе через системный
вызов `exec`, а также добавить её сборку в `Makefile`.

------------------------------------------------------------------------

## 1. Программа `run_sequential.c`

``` c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Использование: %s seed array_size\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Дочерний процесс: запускаем sequential_min_max
        execl("./sequential_min_max", "sequential_min_max", argv[1], argv[2], (char *)NULL);
        perror("execl"); // если дошли сюда, exec не сработал
        exit(1);
    } else {
        // Родитель ждёт завершения дочернего процесса
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("sequential_min_max завершилась с кодом %d\n", WEXITSTATUS(status));
        } else {
            printf("sequential_min_max завершилась аварийно\n");
        }
    }
    return 0;
}
```

------------------------------------------------------------------------

## 2. Обновлённый Makefile

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

------------------------------------------------------------------------

## 3. Использование

### Сборка всех программ

``` bash
make
# или
make all
```

### Запуск последовательной программы напрямую

``` bash
./sequential_min_max 123 1000
```

### Запуск параллельной программы

``` bash
./parallel_min_max --seed 123 --array_size 1000000 --pnum 4
```

### Запуск через `run_sequential` (exec + fork)

``` bash
./run_sequential 123 1000
```

Пример вывода:

    min: 1804289383
    max: 2147483647
    sequential_min_max завершилась с кодом 0

### Очистка бинарников

``` bash
make clean
```

------------------------------------------------------------------------

## 4. Итог

-   Программа `run_sequential.c` демонстрирует работу `fork` + `exec`
    для запуска уже собранного бинарника.
-   В `Makefile` добавлена цель `run_sequential` и цель `all` собирает
    сразу три программы: `sequential_min_max`, `parallel_min_max` и
    `run_sequential`.
