#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <sys/select.h>

#define SADDR struct sockaddr

typedef struct {
    uint32_t seq_num;
    uint32_t ack_num;
    uint16_t flags;
    uint16_t window;
    uint32_t data_len;
    char data[0];
} tcp_packet_t;

#define SYN_FLAG 0x01
#define ACK_FLAG 0x02
#define FIN_FLAG 0x04

void print_usage(const char *program_name) {
    printf("Usage: %s <IPaddress> <port> <buffer_size> <timeout> <max_retries>\n", program_name);
    printf("Example: %s 127.0.0.1 20001 1024 3 5\n", program_name);
}

// Функция для надежной отправки с повторением
int reliable_sendto(int sockfd, tcp_packet_t *packet, size_t packet_size,
                   struct sockaddr *servaddr, socklen_t addr_len, int timeout, int max_retries) {
    int retries = 0;
    fd_set readfds;
    struct timeval tv;
    
    while (retries < max_retries) {
        // Отправка пакета
        if (sendto(sockfd, packet, packet_size, 0, servaddr, addr_len) == -1) {
            perror("sendto problem");
            return -1;
        }
        
        // Ожидание ответа с таймаутом
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        
        int ready = select(sockfd + 1, &readfds, NULL, NULL, &tv);
        
        if (ready > 0) {
            return 0; // Ответ получен
        } else if (ready == 0) {
            printf("Timeout, retrying... (%d/%d)\n", retries + 1, max_retries);
            retries++;
        } else {
            perror("select error");
            return -1;
        }
    }
    
    printf("Max retries exceeded\n");
    return -1;
}

int main(int argc, char **argv) {
    if (argc != 6) {
        print_usage(argv[0]);
        exit(1);
    }

    char *server_ip = argv[1];
    int serv_port = atoi(argv[2]);
    int buf_size = atoi(argv[3]);
    int timeout = atoi(argv[4]);
    int max_retries = atoi(argv[5]);

    if (serv_port <= 0 || buf_size <= 0 || timeout <= 0 || max_retries <= 0) {
        printf("Invalid arguments: all values must be positive integers\n");
        exit(1);
    }

    // Размер пакета с учетом гибкого массива данных
    size_t packet_size = sizeof(tcp_packet_t) + buf_size;
    tcp_packet_t *syn_packet = malloc(packet_size);
    tcp_packet_t *syn_ack = malloc(packet_size);
    tcp_packet_t *data_packet = malloc(packet_size);
    tcp_packet_t *ack_packet = malloc(packet_size);
    tcp_packet_t *fin_packet = malloc(packet_size);
    
    if (!syn_packet || !syn_ack || !data_packet || !ack_packet || !fin_packet) {
        perror("malloc");
        free(syn_packet); free(syn_ack); free(data_packet); free(ack_packet); free(fin_packet);
        exit(1);
    }

    int sockfd, n;
    char *sendline = malloc(buf_size);
    struct sockaddr_in servaddr;
    
    // Состояние соединения
    uint32_t seq_num = 1;
    int connection_established = 0;

    if (!sendline) {
        perror("malloc");
        free(syn_packet); free(syn_ack); free(data_packet); free(ack_packet); free(fin_packet);
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(serv_port);

    if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) < 0) {
        perror("inet_pton problem");
        free(syn_packet); free(syn_ack); free(data_packet); free(ack_packet); free(fin_packet);
        free(sendline);
        exit(1);
    }
    
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket problem");
        free(syn_packet); free(syn_ack); free(data_packet); free(ack_packet); free(fin_packet);
        free(sendline);
        exit(1);
    }

    printf("Connecting to %s:%d with buffer_size=%d, timeout=%d, max_retries=%d\n", 
           server_ip, serv_port, buf_size, timeout, max_retries);

    // Установка соединения (SYN)
    printf("Establishing connection...\n");
    memset(syn_packet, 0, packet_size);
    syn_packet->seq_num = htonl(seq_num);
    syn_packet->flags = htons(SYN_FLAG);
    syn_packet->data_len = 0;
    
    if (reliable_sendto(sockfd, syn_packet, packet_size, (SADDR *)&servaddr, sizeof(servaddr), timeout, max_retries) < 0) {
        printf("Failed to establish connection\n");
        free(syn_packet); free(syn_ack); free(data_packet); free(ack_packet); free(fin_packet);
        free(sendline);
        close(sockfd);
        exit(1);
    }
    
    // Получение SYN-ACK
    if (recvfrom(sockfd, syn_ack, packet_size, 0, NULL, NULL) == -1) {
        perror("recvfrom problem");
        free(syn_packet); free(syn_ack); free(data_packet); free(ack_packet); free(fin_packet);
        free(sendline);
        close(sockfd);
        exit(1);
    }
    
    syn_ack->seq_num = ntohl(syn_ack->seq_num);
    syn_ack->ack_num = ntohl(syn_ack->ack_num);
    syn_ack->flags = ntohs(syn_ack->flags);
    syn_ack->data_len = ntohl(syn_ack->data_len);
    
    if ((syn_ack->flags & (SYN_FLAG | ACK_FLAG)) == (SYN_FLAG | ACK_FLAG)) {
        printf("Connection established\n");
        connection_established = 1;
        seq_num = syn_ack->ack_num;
    } else {
        printf("Failed to establish connection - invalid SYN-ACK\n");
        free(syn_packet); free(syn_ack); free(data_packet); free(ack_packet); free(fin_packet);
        free(sendline);
        close(sockfd);
        exit(1);
    }

    printf("Enter string\n");

    while (connection_established && (n = read(0, sendline, buf_size)) > 0) {
        // Удаление символа новой строки
        if (sendline[n-1] == '\n') {
            sendline[n-1] = '\0';
            n--;
        }
        
        if (n == 0) {
            printf("Empty message, skipping...\n");
            continue;
        }
        
        // Подготовка пакета с данными
        memset(data_packet, 0, packet_size);
        data_packet->seq_num = htonl(seq_num);
        data_packet->flags = 0;
        data_packet->data_len = htonl(n);
        memcpy(data_packet->data, sendline, n);
        
        // Надежная отправка данных
        if (reliable_sendto(sockfd, data_packet, packet_size, (SADDR *)&servaddr, sizeof(servaddr), timeout, max_retries) < 0) {
            printf("Failed to send data\n");
            break;
        }
        
        // Получение ACK
        if (recvfrom(sockfd, ack_packet, packet_size, 0, NULL, NULL) == -1) {
            perror("recvfrom problem");
            break;
        }
        
        ack_packet->seq_num = ntohl(ack_packet->seq_num);
        ack_packet->ack_num = ntohl(ack_packet->ack_num);
        ack_packet->flags = ntohs(ack_packet->flags);
        ack_packet->data_len = ntohl(ack_packet->data_len);
        
        if (ack_packet->flags & ACK_FLAG) {
            printf("REPLY FROM SERVER= %.*s\n", ack_packet->data_len, ack_packet->data);
            seq_num += n;
        }
        
        printf("Enter string\n");
    }

    // Завершение соединения (FIN)
    if (connection_established) {
        printf("Closing connection...\n");
        memset(fin_packet, 0, packet_size);
        fin_packet->seq_num = htonl(seq_num);
        fin_packet->flags = htons(FIN_FLAG);
        fin_packet->data_len = 0;
        
        reliable_sendto(sockfd, fin_packet, packet_size, (SADDR *)&servaddr, sizeof(servaddr), timeout, max_retries);
    }
    
    free(syn_packet); free(syn_ack); free(data_packet); free(ack_packet); free(fin_packet);
    free(sendline);
    close(sockfd);
    return 0;
}