/*
С бинарным семафором:

gcc -o binary_semaphore binary_semaphore.c -pthread
*/

/********************************************************
 * Пример исходного модуля для книги...
 *
 * "Using POSIX Threads: Programming with Pthreads"
 *     автор: Brad Nichols, Dick Buttlar, Jackie Farrell
 *     O'Reilly & Associates, Inc.
 *  Модифицировано A.Kostin
 ********************************************************
 * binary_semaphore.c
 *
 * Пример простой многопоточной программы с бинарным семафором.
 */
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

void do_one_thing(int *);
void do_another_thing(int *);
void do_wrap_up(int);
int common = 0; /* Общая переменная для двух потоков */
int r1 = 0, r2 = 0, r3 = 0;
sem_t bin_sem; /* Бинарный семафор */

int main() {
  pthread_t thread1, thread2;

  if (sem_init(&bin_sem, 0, 1) != 0) {
    perror("sem_init");
    exit(1);
  }

  if (pthread_create(&thread1, NULL, (void *)do_one_thing,
			  (void *)&common) != 0) {
    perror("pthread_create");
    exit(1);
  }

  if (pthread_create(&thread2, NULL, (void *)do_another_thing,
                     (void *)&common) != 0) {
    perror("pthread_create");
    exit(1);
  }

  if (pthread_join(thread1, NULL) != 0) {
    perror("pthread_join");
    exit(1);
  }

  if (pthread_join(thread2, NULL) != 0) {
    perror("pthread_join");
    exit(1);
  }

  do_wrap_up(common);

  sem_destroy(&bin_sem); /* Уничтожение семафора */
  return 0;
}

void do_one_thing(int *pnum_times) {
  int i, j, x;
  unsigned long k;
  int work;
  for (i = 0; i < 50; i++) {
    sem_wait(&bin_sem); /* Захват семафора */
    printf("выполняем одну операцию\n");
    work = *pnum_times;
    printf("счётчик = %d\n", work);
    work++;
    for (k = 0; k < 500000; k++)
      ;
    *pnum_times = work;
	sem_post(&bin_sem); /* Освобождение семафора */
  }
}

void do_another_thing(int *pnum_times) {
  int i, j, x;
  unsigned long k;
  int work;
  for (i = 0; i < 50; i++) {
    sem_wait(&bin_sem); /* Захват семафора */
    printf("выполняем другую операцию\n");
    work = *pnum_times;
    printf("счётчик = %d\n", work);
    work++; /* инкремент без записи */
    for (k = 0; k < 500000; k++)
      ;
    *pnum_times = work;
    sem_post(&bin_sem); /* Освобождение семафора */
  }
}

void do_wrap_up(int counter) {
  int total;
  printf("Всё завершено, счётчик = %d\n", counter);
}