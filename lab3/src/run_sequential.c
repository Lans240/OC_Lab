//./run_sequential 123 1000

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


/* 
int pipe1[2], pipe2[2];
    
    // Создаем два канала
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        return 1;
    }

    // Первый дочерний процесс - запускаем ls
    pid_t pid1 = fork();
    if (pid1 == 0) {
        close(pipe1[0]);  // закрываем чтение
        dup2(pipe1[1], STDOUT_FILENO);  // перенаправляем stdout в канал
        
        execlp("ls", "ls", "-l", "/home", NULL);
        perror("execlp ls failed");
        exit(1);
    }

    // Второй дочерний процесс - запускаем find
    pid_t pid2 = fork();
    if (pid2 == 0) {
        close(pipe2[0]);  // закрываем чтение
        dup2(pipe2[1], STDOUT_FILENO);  // перенаправляем stdout в канал
        
        execlp("find", "find", "/var/log", "-name", "*.log", NULL);
        perror("execlp find failed");
        exit(1);
    }

    // Родительский процесс
    close(pipe1[1]);  // закрываем запись
    close(pipe2[1]);  // закрываем запись*/