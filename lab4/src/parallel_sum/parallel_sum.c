/*
gcc -pthread -o parallel_sum utils.c sum.c parallel_sum.c

# Пример запуска с 4 потоками
./parallel_sum --threads_num 4 --seed 123 --array_size 1000000

# Пример запуска с 2 потоками
./parallel_sum --threads_num 2 --seed 123 --array_size 1000000
*/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <sys/time.h>
#include "utils.h"
#include "sum.h"

void *ThreadSum(void *args) {
  struct SumArgs *sum_args = (struct SumArgs *)args;
  int *result = malloc(sizeof(int));
  *result = Sum(sum_args);
  return (void *)result;
}

// Функция для получения текущего времени
double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

int main(int argc, char **argv) {
  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;

  // Разбор аргументов командной строки
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--threads_num") == 0) {
      threads_num = atoi(argv[++i]);
    } else if (strcmp(argv[i], "--array_size") == 0) {
      array_size = atoi(argv[++i]);
    } else if (strcmp(argv[i], "--seed") == 0) {
      seed = atoi(argv[++i]);
    }
  }

  if (threads_num == 0 || array_size == 0) {
    printf("Использование: %s --threads_num <num> --seed <num> --array_size <num>\n", argv[0]);
    return 1;
  }

  // Генерация массива
  int *array = malloc(sizeof(int) * array_size);
  if (array == NULL) {
    printf("Ошибка выделения памяти для массива\n");
    return 1;
  }
  GenerateArray(array, array_size, seed);

  // Выделение памяти для потоков и аргументов
  pthread_t *threads = malloc(sizeof(pthread_t) * threads_num);
  struct SumArgs *args = malloc(sizeof(struct SumArgs) * threads_num);
  
  if (threads == NULL || args == NULL) {
    printf("Ошибка выделения памяти для потоков\n");
    free(array);
    return 1;
  }

  int segment_size = array_size / threads_num;

  // Начало замера времени (только суммирование, без генерации массива)
  double start_time = get_time();

  // Создание потоков
  for (uint32_t i = 0; i < threads_num; i++) {
    args[i].array = array;
    args[i].begin = i * segment_size;
    // Последний поток берет оставшиеся элементы
    args[i].end = (i == threads_num - 1) ? array_size : (i + 1) * segment_size;

    if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])) {
      printf("Ошибка: не удалось создать поток!\n");
      free(array);
      free(threads);
      free(args);
      return 1;
    }
  }

  // Сбор результатов
  int total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    int *sum_ptr = NULL;
    pthread_join(threads[i], (void **)&sum_ptr);
    if (sum_ptr != NULL) {
      total_sum += *sum_ptr;
      free(sum_ptr);
    }
  }

  // Окончание замера времени
  double end_time = get_time();
  double time_taken = end_time - start_time;

  free(array);
  free(threads);
  free(args);
  printf("Сумма: %d\n", total_sum);
  printf("Время: %.6f секунд\n", time_taken);
  return 0;
}