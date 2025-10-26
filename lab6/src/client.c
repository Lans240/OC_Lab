/*
Запуск серверов
Терминал 1: ./server --port 20001 --tnum 2
Терминал 2: ./server --port 20002 --tnum 2
Терминал 3: ./server --port 20003 --tnum 2

Запуск клиента
Терминал 4: ./client --k 20 --mod 100000 --servers servers.txt
*/

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
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

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

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
  uint64_t result = 0;
  a = a % mod;
  while (b > 0) {
    if (b % 2 == 1)
      result = (result + a) % mod;
    a = (a * 2) % mod;
    b /= 2;
  }
  return result % mod;
}

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
  
  struct hostent *hostname = gethostbyname(data->server.ip);
  if (hostname == NULL) {
    fprintf(stderr, "gethostbyname failed with %s\n", data->server.ip);
    data->success = 0;
    pthread_exit(NULL);
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(data->server.port);
  
  // Правильное копирование адреса
  server.sin_addr = *((struct in_addr *)hostname->h_addr_list[0]);

  int sck = socket(AF_INET, SOCK_STREAM, 0);
  if (sck < 0) {
    fprintf(stderr, "Socket creation failed!\n");
    data->success = 0;
    pthread_exit(NULL);
  }

  if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
    fprintf(stderr, "Connection to %s:%d failed\n", data->server.ip, data->server.port);
    close(sck);
    data->success = 0;
    pthread_exit(NULL);
  }

  char task[sizeof(uint64_t) * 3];
  memcpy(task, &data->begin, sizeof(uint64_t));
  memcpy(task + sizeof(uint64_t), &data->end, sizeof(uint64_t));
  memcpy(task + 2 * sizeof(uint64_t), &data->mod, sizeof(uint64_t));

  if (send(sck, task, sizeof(task), 0) < 0) {
    fprintf(stderr, "Send failed to %s:%d\n", data->server.ip, data->server.port);
    close(sck);
    data->success = 0;
    pthread_exit(NULL);
  }

  char response[sizeof(uint64_t)];
  if (recv(sck, response, sizeof(response), 0) < 0) {
    fprintf(stderr, "Receive failed from %s:%d\n", data->server.ip, data->server.port);
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
        memcpy(servers_file, optarg, strlen(optarg));
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

  // Read servers from file
  FILE* file = fopen(servers_file, "r");
  if (file == NULL) {
    fprintf(stderr, "Cannot open servers file: %s\n", servers_file);
    return 1;
  }

  struct Server* servers = NULL;
  int servers_num = 0;
  char line[255];
  
  while (fgets(line, sizeof(line), file)) {
    char ip[255];
    int port;
    if (sscanf(line, "%254[^:]:%d", ip, &port) == 2) {
      servers = realloc(servers, (servers_num + 1) * sizeof(struct Server));
      strcpy(servers[servers_num].ip, ip);
      servers[servers_num].port = port;
      servers_num++;
    }
  }
  fclose(file);

  if (servers_num == 0) {
    fprintf(stderr, "No servers found in file\n");
    free(servers);
    return 1;
  }

  printf("Found %d servers\n", servers_num);

  // Split work between servers
  pthread_t threads[servers_num];
  struct ThreadData thread_data[servers_num];
  
  uint64_t range_size = k / servers_num;
  uint64_t remainder = k % servers_num;
  
  for (int i = 0; i < servers_num; i++) {
    thread_data[i].server = servers[i];
    thread_data[i].mod = mod;
    
    thread_data[i].begin = i * range_size + 1;
    thread_data[i].end = (i + 1) * range_size;
    
    // Distribute remainder to the last server
    if (i == servers_num - 1) {
      thread_data[i].end += remainder;
    }
    
    printf("Server %d: %s:%d will compute [%" PRIu64 "-%" PRIu64 "]\n", 
           i, servers[i].ip, servers[i].port, thread_data[i].begin, thread_data[i].end);
    
    if (pthread_create(&threads[i], NULL, ProcessServer, &thread_data[i])) {
      fprintf(stderr, "Error creating thread for server %d\n", i);
    }
  }

  // Wait for all threads to complete
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