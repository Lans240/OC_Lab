// gcc -Wall -Wextra -std=c11 -O2 parallel_min_max.c find_min_max.c utils.c -o parallel

/*
# режим через каналы (pipe)
./parallel --seed 123 --array_size 1000000 --pnum 4

# режим через файлы
./parallel --seed 123 --array_size 1000000 --pnum 4 --by_files
# или коротко
./parallel --seed 123 --array_size 1000000 --pnum 4 -f
*/

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>
#include <errno.h>

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;

  while (true) {

    static struct option options[] = {
        {"seed", required_argument, 0, 0},
        {"array_size", required_argument, 0, 0},
        {"pnum", required_argument, 0, 0},
        {"by_files", no_argument, 0, 'f'},
        {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            break;
          case 1:
            array_size = atoi(optarg);
            break;
          case 2:
            pnum = atoi(optarg);
            break;
          case 3:
            with_files = true;
            break;
          default:
            printf("Индекс %d вне опций\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;
      case '?':
        break;
      default:
        printf("getopt вернул неожиданный код 0%o\n", c);
    }
  }

  if (optind < argc) {
    printf("Есть хотя бы один позиционный аргумент без опции\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Использование: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--by_files]\n",
           argv[0]);
    return 1;
  }

  if (seed <= 0 || array_size <= 0 || pnum <= 0) {
    printf("Параметры seed, array_size и pnum должны быть положительными числами\n");
    return 1;
  }

  if (pnum > array_size) {
    // не имеет смысла запускать больше процессов, чем элементов
    pnum = array_size;
  }

  int *array = malloc(sizeof(int) * array_size);
  if (array == NULL) {
    perror("malloc");
    return 1;
  }
  GenerateArray(array, array_size, seed);

  // pipes (если не with_files)
  int (*pipes)[2] = NULL;
  if (!with_files) {
    pipes = malloc(sizeof(int[2]) * pnum);
    if (!pipes) {
      perror("malloc для pipe");
      free(array);
      return 1;
    }
    for (int i = 0; i < pnum; ++i) {
      if (pipe(pipes[i]) == -1) {
        perror("pipe");
        free(array);
        free(pipes);
        return 1;
      }
    }
  }

  int active_child_processes = 0;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // успешный fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // дочерний процесс
        int chunk_size = array_size / pnum;
        int remainder = array_size % pnum;
        int begin = i * chunk_size + (i < remainder ? i : remainder);
        int end = begin + chunk_size + (i < remainder ? 1 : 0);

        struct MinMax local = GetMinMax(array, (unsigned int)begin, (unsigned int)end);

        if (with_files) {
          char filename[64];
          snprintf(filename, sizeof(filename), "minmax_%d.txt", i);
          FILE *f = fopen(filename, "w");
          if (!f) {
            fprintf(stderr, "Дочерний процесс %d: не удалось открыть файл %s: %s\n", i, filename, strerror(errno));
            _exit(1);
          }
          fprintf(f, "%d %d\n", local.min, local.max);
          fclose(f);
        } else {
          // записываем два int в pipe
          close(pipes[i][0]); // закрываем конец для чтения
          int buf[2];
          buf[0] = local.min;
          buf[1] = local.max;
          ssize_t written = write(pipes[i][1], buf, sizeof(buf));
          if (written != sizeof(buf)) {
            fprintf(stderr, "Дочерний процесс %d: запись в канал не удалась\n", i);
          }
          close(pipes[i][1]);
        }
        free(array);
        if (pipes) free(pipes);
        _exit(0);
      }
      // родитель продолжает цикл
    } else {
      fprintf(stderr, "Ошибка fork!\n");
      if (pipes) free(pipes);
      free(array);
      return 1;
    }
  }

  // родитель: закрываем ненужные дескрипторы
  if (!with_files) {
    // родитель не будет писать; закрываем все write-эндпоинты
    for (int i = 0; i < pnum; ++i) {
      close(pipes[i][1]);
    }
  }

  // ожидаем дочерние процессы
  while (active_child_processes > 0) {
    wait(NULL);
    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      char filename[64];
      snprintf(filename, sizeof(filename), "minmax_%d.txt", i);
      FILE *f = fopen(filename, "r");
      if (!f) {
        fprintf(stderr, "Родитель: не удалось открыть файл %s для чтения: %s\n", filename, strerror(errno));
        continue;
      }
      if (fscanf(f, "%d %d", &min, &max) != 2) {
        fprintf(stderr, "Родитель: не удалось прочитать данные из файла %s\n", filename);
        fclose(f);
        continue;
      }
      fclose(f);
      // можно удалить файл при желании:
      // remove(filename);
    } else {
      int buf[2];
      ssize_t r = read(pipes[i][0], buf, sizeof(buf));
      if (r == sizeof(buf)) {
        min = buf[0];
        max = buf[1];
      } else {
        fprintf(stderr, "Родитель: чтение из канала %d не удалось (r=%zd)\n", i, r);
      }
      close(pipes[i][0]);
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  if (pipes) free(pipes);
  free(array);

  printf("Минимум: %d\n", min_max.min);
  printf("Максимум: %d\n", min_max.max);
  printf("Время выполнения: %f мс\n", elapsed_time);
  fflush(NULL);
  return 0;
}
