//gcc -o factorial factorial.c -pthread
//./factorial -k 10 --pnum=4 --mod=100
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>

// Структура для передачи данных в поток
typedef struct {
    int start;
    int end;
    long long mod;
    long long partial_result; // частичный результат потока
} thread_data_t;

// Глобальные переменные
long long result = 1;   // общий результат
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // мьютекс для синхронизации

// Функция, которую выполняет каждый поток
void* calculate_partial_factorial(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    data->partial_result = 1;

    // Вычисляем частичный факториал для своего диапазона
    for (int i = data->start; i <= data->end; ++i) {
        data->partial_result = (data->partial_result * i) % data->mod;
    }

    // Синхронизируем доступ к общему результату
    pthread_mutex_lock(&mutex);
    result = (result * data->partial_result) % data->mod;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char* argv[]) {
    int k = 0;
    int pnum = 0;
    long long mod = 0;

    // Опции командной строки
    struct option long_options[] = {
        {"k", required_argument, 0, 'k'},
        {"pnum", required_argument, 0, 'p'},
        {"mod", required_argument, 0, 'm'},
        {0, 0, 0, 0}
    };

    // Разбор аргументов командной строки
    int opt;
    while ((opt = getopt_long(argc, argv, "k:p:m:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'k': k = atoi(optarg); break;
            case 'p': pnum = atoi(optarg); break;
            case 'm': mod = atoll(optarg); break;
            default: 
                fprintf(stderr, "Использование: %s -k <число> --pnum=<потоки> --mod=<модуль>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Проверка корректности входных данных
    if (k <= 0 || pnum <= 0 || mod <= 0) {
        fprintf(stderr, "Ошибка: все параметры должны быть положительными числами\n");
        exit(EXIT_FAILURE);
    }

    // Создание массивов для потоков и их данных
    pthread_t threads[pnum];
    thread_data_t thread_data[pnum];
    
    // Распределение работы между потоками
    int step = k / pnum;        // базовый размер диапазона для потока
    int remainder = k % pnum;   // остаток для распределения

    printf("Распределение работы:\n");
    for (int i = 0; i < pnum; ++i) {
        thread_data[i].start = i * step + 1;
        thread_data[i].end = (i + 1) * step;
        thread_data[i].mod = mod;

        // Последний поток получает дополнительный остаток
        if (i == pnum - 1) {
            thread_data[i].end += remainder;
        }

        printf("  Поток %d: числа от %d до %d\n", i, thread_data[i].start, thread_data[i].end);
    }

    // Создание потоков
    printf("\nСоздание %d потоков...\n", pnum);
    for (int i = 0; i < pnum; ++i) {
        if (pthread_create(&threads[i], NULL, calculate_partial_factorial, &thread_data[i]) != 0) {
            perror("Ошибка при создании потока");
            exit(EXIT_FAILURE);
        }
    }

    // Ожидание завершения всех потоков
    printf("Ожидание завершения потоков...\n");
    for (int i = 0; i < pnum; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Вывод результата
    printf("\nРезультат вычислений:\n");
    printf("%d! mod %lld = %lld\n", k, mod, result);

    // Вывод промежуточных результатов для наглядности
    printf("\nЧастичные результаты потоков:\n");
    for (int i = 0; i < pnum; ++i) {
        printf("  Поток %d: %lld\n", i, thread_data[i].partial_result);
    }

    // Освобождение ресурсов мьютекса
    pthread_mutex_destroy(&mutex);
    
    return 0;
}