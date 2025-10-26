//gcc -o deadlock deadlock.c -pthread

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void* thread1_function(void* arg) {
    pthread_mutex_lock(&mutex1);
    printf("Поток 1: захватил мьютекс 1\n");
    sleep(1); // Имитация работы
    printf("Поток 1: пытается захватить мьютекс 2\n");
    pthread_mutex_lock(&mutex2); // Тут произойдет дедлок
    printf("Поток 1: захватил мьютекс 2\n");
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    return NULL;
}

void* thread2_function(void* arg) {
    pthread_mutex_lock(&mutex2);
    printf("Поток 2: захватил мьютекс 2\n");
    sleep(1); // Имитация работы
    printf("Поток 2: пытается захватить мьютекс 1\n");
    pthread_mutex_lock(&mutex1); // Тут произойдет дедлок
    printf("Поток 2: захватил мьютекс 1\n");
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    pthread_create(&thread1, NULL, thread1_function, NULL);
    pthread_create(&thread2, NULL, thread2_function, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Программа завершена (этого не должно быть)\n");
    return 0;
}