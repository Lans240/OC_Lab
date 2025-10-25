//gcc zombie.c -o zombie
//./zombie

//в другом терминале: ps -e -o pid,ppid,state,command | grep <PID_дочернего_процесса>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork(); // Создаем дочерний процесс

    if (pid == 0) {
        // Дочерний процесс
        printf("Дочерний PID: %d\n", getpid());
        exit(0); // Немедленное завершение
    } else if (pid > 0) {
        // Родительский процесс
        printf("Родительский PID: %d\n", getpid());
        printf("Дочерний процесс создан с PID: %d\n", pid);
        printf("Ожидание 30 секунд...\n");
        sleep(30); // Ждем 30 секунд, не собирая статус
        //wait(NULL); // Собрать статус завершения дочернего процесса
        printf("Родитель завершает работу.\n");
    } else {
        perror("Ошибка fork");
        return 1;
    }

    return 0;
}