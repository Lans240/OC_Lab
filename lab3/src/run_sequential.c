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
        // Дочерний процесс: заменяем образ процесса на sequential_min_max
        execl("./sequential_min_max", "sequential_min_max", argv[1], argv[2], (char *)NULL);
        // если execl вернулся, значит ошибка
        perror("execl");
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
/*Что делает программа:

Проверяет, что передано 2 аргумента (seed и array_size).

Создаёт новый процесс с fork().

В дочернем процессе вызывает execl("./sequential_min_max", ...) → запускает вашу программу.

Родитель ждёт завершения и выводит сообщение о коде возврата.*/