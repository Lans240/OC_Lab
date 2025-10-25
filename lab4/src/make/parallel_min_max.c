/*
1. Компиляция: 

gcc -std=c11 -O2 parallel_min_max.c find_min_max.c utils.c -o parallel_min_max

2. Тестирование с таймаутом: 

./parallel_min_max --seed 42 --array_size 50000000 --pnum 4 --timeout 3

3. Тестирование без таймаута: 

./parallel_min_max --seed 42 --array_size 1000000 --pnum 2

4. Тестирование с коротким таймаутом: 

./parallel_min_max --seed 42 --array_size 100000000 --pnum 4 -t 1
*/

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>

#include "find_min_max.h"
#include "utils.h"

// Флаг таймаута
int timeout_flag = 0;
int timeout_seconds = 0;

// Простой обработчик сигнала
void handle_timeout(int sig) {
    (void)sig;
    timeout_flag = 1;
    printf("\nТАЙМАУТ! Сработал сигнал SIGALRM после %d секунд\n", timeout_seconds);
}

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    int timeout = -1;

    // Простая обработка аргументов
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--array_size") == 0 && i + 1 < argc) {
            array_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--pnum") == 0 && i + 1 < argc) {
            pnum = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--timeout") == 0 && i + 1 < argc) {
            timeout = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            timeout = atoi(argv[++i]);
        }
    }

    // Проверка параметров
    if (seed == -1 || array_size == -1 || pnum == -1) {
        printf("Usage: %s --seed NUM --array_size NUM --pnum NUM [--timeout SECONDS]\n", argv[0]);
        return 1;
    }

    // Сохраняем таймаут для вывода
    timeout_seconds = timeout;

    // Создание массива
    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);

    printf("Параметры:\n");
    printf("  - Размер массива: %d\n", array_size);
    printf("  - Количество процессов: %d\n", pnum);
    if (timeout > 0) {
        printf("  - Таймаут: %d секунд\n", timeout);
        printf("Таймер запущен...\n");
    } else {
        printf("  - Таймаут: не установлен\n");
    }
    printf("\n");

    // Создание pipe'ов с проверкой ошибок
    int (*pipes)[2] = malloc(sizeof(int[2]) * pnum);
    for (int i = 0; i < pnum; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    pid_t *child_pids = malloc(sizeof(pid_t) * pnum);
    
    // Запуск дочерних процессов
    printf("Запуск %d дочерних процессов...\n", pnum);
    for (int i = 0; i < pnum; i++) {
        pid_t child_pid = fork();
        if (child_pid == 0) {
            // Дочерний процесс
            int chunk_size = array_size / pnum;
            int begin = i * chunk_size;
            int end = (i == pnum - 1) ? array_size : begin + chunk_size;

            struct MinMax local = GetMinMax(array, begin, end);

            // Запись в pipe с проверкой ошибок
            close(pipes[i][0]);
            int buf[2] = {local.min, local.max};
            ssize_t written = write(pipes[i][1], buf, sizeof(buf));
            if (written != sizeof(buf)) {
                exit(1);
            }
            close(pipes[i][1]);
            
            free(array);
            free(pipes);
            free(child_pids);
            exit(0);
        } else {
            // Родительский процесс
            child_pids[i] = child_pid;
            printf("  Процесс %d запущен (PID: %d)\n", i, child_pid);
        }
    }

    // Закрытие ненужных концов pipe'ов
    for (int i = 0; i < pnum; i++) {
        close(pipes[i][1]);
    }

    // Установка таймаута
    if (timeout > 0) {
        signal(SIGALRM, handle_timeout);
        alarm(timeout);
    }

    // Ожидание завершения дочерних процессов
    printf("\nОжидание завершения процессов");
    fflush(stdout);
    
    int active_children = pnum;
    int dots = 0;
    
    while (active_children > 0) {
        int status;
        pid_t finished_pid = waitpid(-1, &status, WNOHANG);
        
        if (finished_pid > 0) {
            active_children--;
            printf("\n  Процесс %d завершился\n", finished_pid);
        } else if (timeout_flag) {
            // Таймаут! Завершаем все дочерние процессы
            printf("\nОтправка SIGKILL оставшимся процессам...\n");
            for (int i = 0; i < pnum; i++) {
                if (child_pids[i] > 0) {
                    kill(child_pids[i], SIGKILL);
                    printf("  Процесс %d убит\n", child_pids[i]);
                }
            }
            break;
        }
        
        // Простой индикатор прогресса
        printf(".");
        fflush(stdout);
        dots++;
        if (dots > 3) {
            dots = 0;
            printf("\rОжидание завершения процессов   ");
            fflush(stdout);
        }
        
        // Пауза
        struct timespec sleep_time = {0, 500000000}; // 0.5 секунды
        nanosleep(&sleep_time, NULL);
    }

    // Отмена таймера
    if (timeout > 0 && !timeout_flag) {
        alarm(0);
        printf("\nВсе процессы завершились до истечения таймаута\n");
    }

    // Сбор результатов
    printf("\nСбор результатов...\n");
    struct MinMax min_max = {INT_MAX, INT_MIN};
    int successful_reads = 0;

    for (int i = 0; i < pnum; i++) {
        int buf[2];
        ssize_t bytes_read = read(pipes[i][0], buf, sizeof(buf));
        
        if (bytes_read == sizeof(buf)) {
            if (buf[0] < min_max.min) min_max.min = buf[0];
            if (buf[1] > min_max.max) min_max.max = buf[1];
            successful_reads++;
        }
        close(pipes[i][0]);
    }

    // Вывод результатов
    printf("\nРЕЗУЛЬТАТЫ:\n");
    printf("  Минимум: %d\n", min_max.min);
    printf("  Максимум: %d\n", min_max.max);
    printf("  Успешно обработано процессов: %d/%d\n", successful_reads, pnum);
    
    if (timeout_flag) {
        printf("  Программа была прервана по таймауту\n");
        printf("  Результаты могут быть неполными\n");
    } else {
        printf("  Все процессы завершились успешно\n");
    }

    // Освобождение памяти
    free(array);
    free(pipes);
    free(child_pids);

    return 0;
}