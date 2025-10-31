/*
Запуск серверов
Терминал 1: ./server --port 20001 --tnum 2
Терминал 2: ./server --port 20002 --tnum 2
Терминал 3: ./server --port 20003 --tnum 2

Запуск клиента
Терминал 4: ./client --k 20 --mod 100000 --servers servers.txt
*/

#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <inttypes.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "multmodulo.h"

struct Server {
  char ip[255];
  int port;
};

struct ThreadData {
  struct Server server;
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
  uint64_t result;
  int success;
};

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }
  if (errno != 0)
    return false;
  *val = i;
  return true;
}

void* ProcessServer(void* arg) {
  struct ThreadData* data = (struct ThreadData*)arg;
  
  // Создание сокета и подключение
  int sck = socket(AF_INET6, SOCK_STREAM, 0);
  if (sck < 0) {
    fprintf(stderr, "Socket creation failed!\n");
    data->success = 0;
    pthread_exit(NULL);
  }

  // Настройка структуры адреса сервера
  struct sockaddr_in6 server;
  memset(&server, 0, sizeof(server));
  server.sin6_family = AF_INET6;
  server.sin6_port = htons(data->server.port);
  
  // Преобразование IPv6 адреса из текстового формата в бинарный
  if (inet_pton(AF_INET6, data->server.ip, &server.sin6_addr) <= 0) {
    fprintf(stderr, "Invalid address: %s\n", data->server.ip);
    close(sck);
    data->success = 0;
    pthread_exit(NULL);
  }

  // Установка таймаута на подключение
  struct timeval timeout;
  timeout.tv_sec = 5;
  timeout.tv_usec = 0;
  setsockopt(sck, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
  setsockopt(sck, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
    fprintf(stderr, "Connection to %s:%d failed: %s\n", 
            data->server.ip, data->server.port, strerror(errno));
    close(sck);
    data->success = 0;
    pthread_exit(NULL);
  }

  // Отправка задачи серверу
  char task[sizeof(uint64_t) * 3];
  memcpy(task, &data->begin, sizeof(uint64_t));
  memcpy(task + sizeof(uint64_t), &data->end, sizeof(uint64_t));
  memcpy(task + 2 * sizeof(uint64_t), &data->mod, sizeof(uint64_t));

  if (send(sck, task, sizeof(task), 0) < 0) {
    fprintf(stderr, "Send failed to %s:%d: %s\n", 
            data->server.ip, data->server.port, strerror(errno));
    close(sck);
    data->success = 0;
    pthread_exit(NULL);
  }

  // Получение результата
  char response[sizeof(uint64_t)];
  int bytes_received = recv(sck, response, sizeof(response), 0);
  if (bytes_received < (int)sizeof(uint64_t)) {
    fprintf(stderr, "Receive failed from %s:%d, received %d bytes: %s\n", 
            data->server.ip, data->server.port, bytes_received, strerror(errno));
    close(sck);
    data->success = 0;
    pthread_exit(NULL);
  }

  memcpy(&data->result, response, sizeof(uint64_t));
  close(sck);
  data->success = 1;
  
  printf("Server %s:%d computed range [%" PRIu64 "-%" PRIu64 "] = %" PRIu64 "\n", 
         data->server.ip, data->server.port, data->begin, data->end, data->result);
  
  pthread_exit(NULL);
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers_file[255] = {'\0'};

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        ConvertStringToUI64(optarg, &k);
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        break;
      case 2:
        strncpy(servers_file, optarg, sizeof(servers_file) - 1);
        servers_file[sizeof(servers_file) - 1] = '\0';
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;
    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers_file)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n", argv[0]);
    return 1;
  }

  printf("Looking for servers file: %s\n", servers_file);

  FILE* file = fopen(servers_file, "r");
  if (file == NULL) {
    fprintf(stderr, "Cannot open servers file: %s, error: %s\n", servers_file, strerror(errno));
    return 1;
  }

  struct Server* servers = NULL;
  int servers_num = 0;
  char line[255];
  
  while (fgets(line, sizeof(line), file)) {
    // Убираем символ новой строки
    line[strcspn(line, "\n")] = 0;
    
    // Пропускаем пустые строки и комментарии
    if (strlen(line) == 0 || line[0] == '#')
      continue;
    
    char ip[255] = "";
    int port = 0;
    
    // Пробуем разные форматы парсинга
    int parsed = 0;
    
    // Формат 1: [ip]:port (IPv6 с квадратными скобками)
    if (sscanf(line, "[%254[^]]]:%d", ip, &port) == 2) {
      parsed = 1;
    }
    // Формат 2: ip:port (IPv4 или IPv6 без скобок)
    else if (sscanf(line, "%254[^:]:%d", ip, &port) == 2) {
      parsed = 1;
    }
    // Формат 3: попробуем найти последнее двоеточие для порта
    else {
      char *last_colon = strrchr(line, ':');
      if (last_colon != NULL) {
        char *port_str = last_colon + 1;
        // Проверяем, что после последнего двоеточия есть цифры
        if (*port_str != '\0') {
          char *endptr;
          long port_long = strtol(port_str, &endptr, 10);
          if (*endptr == '\0' && port_long > 0 && port_long <= 65535) {
            // Копируем IP часть (все до последнего двоеточия)
            size_t ip_len = last_colon - line;
            if (ip_len < sizeof(ip)) {
              strncpy(ip, line, ip_len);
              ip[ip_len] = '\0';
              port = (int)port_long;
              parsed = 1;
            }
          }
        }
      }
    }
    
    if (!parsed) {
      fprintf(stderr, "Invalid server line format: %s\n", line);
      continue;
    }
    
    servers = realloc(servers, (servers_num + 1) * sizeof(struct Server));
    strncpy(servers[servers_num].ip, ip, sizeof(servers[servers_num].ip) - 1);
    servers[servers_num].ip[sizeof(servers[servers_num].ip) - 1] = '\0';
    servers[servers_num].port = port;
    servers_num++;
    
    printf("Added server: %s:%d\n", ip, port);
  }
  fclose(file);

  if (servers_num == 0) {
    fprintf(stderr, "No valid servers found in file: %s\n", servers_file);
    fprintf(stderr, "File format should be: [ip]:port or ip:port, one per line\n");
    free(servers);
    return 1;
  }

  printf("Found %d servers\n", servers_num);

  pthread_t threads[servers_num];
  struct ThreadData thread_data[servers_num];
  
  uint64_t range_size = k / servers_num;
  uint64_t remainder = k % servers_num;
  
  for (int i = 0; i < servers_num; i++) {
    thread_data[i].server = servers[i];
    thread_data[i].mod = mod;
    
    thread_data[i].begin = i * range_size + 1;
    thread_data[i].end = (i + 1) * range_size;
    
    if (i == servers_num - 1) {
      thread_data[i].end += remainder;
    }
    
    printf("Server %d: %s:%d will compute [%" PRIu64 "-%" PRIu64 "]\n", 
           i, servers[i].ip, servers[i].port, thread_data[i].begin, thread_data[i].end);
    
    if (pthread_create(&threads[i], NULL, ProcessServer, &thread_data[i])) {
      fprintf(stderr, "Error creating thread for server %d\n", i);
    }
  }

  uint64_t total_result = 1;
  for (int i = 0; i < servers_num; i++) {
    pthread_join(threads[i], NULL);
    if (thread_data[i].success) {
      total_result = MultModulo(total_result, thread_data[i].result, mod);
    } else {
      fprintf(stderr, "Server %s:%d failed\n", servers[i].ip, servers[i].port);
    }
  }

  printf("\nFinal result: %" PRIu64 "! mod %" PRIu64 " = %" PRIu64 "\n", k, mod, total_result);
  
  free(servers);
  return 0;
}