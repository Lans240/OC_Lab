//gcc -o alarm alarm.c
//./alarm

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

// Пользовательский обработчик для SIGALRM
void alarm_handler(int signum) {
    printf ("Будильник сработал! Получен сигнал %d.\n", signum);
    // Здесь можно выполнить какие-то действия по таймауту
    exit(0); // Завершаем работу
}

int main() {
    // Устанавливаем наш обработчик для SIGALRM
    if (signal(SIGALRM, alarm_handler) == SIG_ERR) {
        perror("Невозможно установить обработчик сигнала");
        return 1;
    }

    printf("Устанавливаем будильник на 5 секунд.\n");
    alarm(5); // Заводим будильник

    // Бесконечный цикл, который прервет сигнал
    while(1) {
        printf("Ожидание... (PID: %d)\n", getpid());
        sleep(1); // Имитация работы
    }

    return 0;
}