/*
Запуск серверов
Терминал 1: ./server --port 20001 --tnum 2
Терминал 2: ./server --port 20002 --tnum 2
Терминал 3: ./server --port 20003 --tnum 2

Запуск клиента
Терминал 4: ./client --k 20 --mod 100000 --servers servers.txt
*/

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <arpa/inet.h>

#include <getopt.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "pthread.h"
#include "multmodulo.h"

struct FactorialArgs {
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
};

uint64_t Factorial(const struct FactorialArgs *args) {
  uint64_t ans = 1;
  
  for (uint64_t i = args->begin; i <= args->end; i++) {
    ans = MultModulo(ans, i, args->mod);
  }
  
  printf("Thread computed [%" PRIu64 "-%" PRIu64 "] mod %" PRIu64 " = %" PRIu64 "\n", 
         args->begin, args->end, args->mod, ans);
  return ans;
}

void *ThreadFactorial(void *args) {
  struct FactorialArgs *fargs = (struct FactorialArgs *)args;
  uint64_t *result = malloc(sizeof(uint64_t));
  *result = Factorial(fargs);
  return (void *)result;
}

int main(int argc, char **argv) {
  int tnum = -1;
  int port = -1;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"port", required_argument, 0, 0},
                                      {"tnum", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        port = atoi(optarg);
        break;
      case 1:
        tnum = atoi(optarg);
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;
    case '?':
      printf("Unknown argument\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (port == -1 || tnum == -1) {
    fprintf(stderr, "Using: %s --port 20001 --tnum 4\n", argv[0]);
    return 1;
  }

  // Создание серверного сокета
  int server_fd = socket(AF_INET6, SOCK_STREAM, 0);
  if (server_fd < 0) {
    fprintf(stderr, "Can not create server socket!");
    return 1;
  }
  // Настройка адреса сервера
  struct sockaddr_in6 server;
  memset(&server, 0, sizeof(server)); // Обнуление структуры
  server.sin6_family = AF_INET6; // IPv6
  server.sin6_port = htons((uint16_t)port);  // Порт в сетевом порядке байт
  server.sin6_addr = in6addr_any; // Любой IP-адрес

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

  // Привязка сокета к адресу
  int err = bind(server_fd, (struct sockaddr *)&server, sizeof(server));
  if (err < 0) {
    fprintf(stderr, "Can not bind to socket!");
    return 1;
  }
  
  //Режим прослушивания
  err = listen(server_fd, 128);
  if (err < 0) {
    fprintf(stderr, "Could not listen on socket\n");
    return 1;
  }

  printf("Server listening at [::]:%d with %d threads\n", port, tnum);

  while (true) {
    struct sockaddr_in6 client;
    socklen_t client_len = sizeof(client);
    // Принятие входящего соединения
    int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

    if (client_fd < 0) {
      fprintf(stderr, "Could not establish new connection\n");
      continue;
    }

    // Получение и вывод IP-адреса клиента
    char client_ip[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &client.sin6_addr, client_ip, INET6_ADDRSTRLEN);
    printf("Client connected from %s:%d\n", client_ip, ntohs(client.sin6_port));

    while (true) {
      unsigned int buffer_size = sizeof(uint64_t) * 3;
      char from_client[buffer_size];
      int read_bytes = recv(client_fd, from_client, buffer_size, 0);

      if (!read_bytes)
        break;
      if (read_bytes < 0) {
        fprintf(stderr, "Client read failed\n");
        break;
      }
      if (read_bytes < buffer_size) {
        fprintf(stderr, "Client send wrong data format\n");
        break;
      }

      pthread_t threads[tnum];

      uint64_t begin = 0;
      uint64_t end = 0;
      uint64_t mod = 0;
      memcpy(&begin, from_client, sizeof(uint64_t));
      memcpy(&end, from_client + sizeof(uint64_t), sizeof(uint64_t));
      memcpy(&mod, from_client + 2 * sizeof(uint64_t), sizeof(uint64_t));

      printf("Received task: compute [%" PRIu64 "-%" PRIu64 "] mod %" PRIu64 "\n", begin, end, mod);

      struct FactorialArgs args[tnum];
      
      uint64_t range = end - begin + 1;
      uint64_t step = range / tnum;
      uint64_t remainder = range % tnum;
      
      for (uint32_t i = 0; i < tnum; i++) {
        args[i].begin = begin + i * step;
        args[i].end = begin + (i + 1) * step - 1;
        
        if (i == tnum - 1) {
          args[i].end += remainder;
        }
        
        args[i].mod = mod;
        
        if (pthread_create(&threads[i], NULL, ThreadFactorial, (void *)&args[i])) {
          printf("Error: pthread_create failed!\n");
          return 1;
        }
      }

      uint64_t total = 1;
      for (uint32_t i = 0; i < tnum; i++) {
        uint64_t *result_ptr;
        pthread_join(threads[i], (void **)&result_ptr);
        total = MultModulo(total, *result_ptr, mod);
        free(result_ptr);
      }

      printf("Server computed total: %" PRIu64 "\n", total);

      char buffer[sizeof(total)];
      memcpy(buffer, &total, sizeof(total));
      err = send(client_fd, buffer, sizeof(total), 0);
      if (err < 0) {
        fprintf(stderr, "Can't send data to client\n");
        break;
      }
      break;
    }

    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
  }

  return 0;
}