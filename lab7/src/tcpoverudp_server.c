#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>

#define SADDR struct sockaddr


typedef struct {
    uint32_t seq_num;     // порядковый номер
    uint32_t ack_num;     // номер подтверждения
    uint16_t flags;       // флаги (SYN, ACK, FIN)
    uint16_t window;      // размер окна
    uint32_t data_len;    // длина данных
    char data[0];         // гибкий массив данных
} tcp_packet_t;

#define SYN_FLAG 0x01  // Установка соединения
#define ACK_FLAG 0x02  // Подтверждение  
#define FIN_FLAG 0x04  // Завершение соединения

void print_usage(const char *program_name) {
    printf("Usage: %s <port> <buffer_size> <packet_loss_percentage>\n", program_name);
    printf("Example: %s 20001 1024 20\n", program_name);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        print_usage(argv[0]);
        exit(1);
    }

    int serv_port = atoi(argv[1]);
    int buf_size = atoi(argv[2]);
    int packet_loss_percentage = atoi(argv[3]);

    if (serv_port <= 0 || buf_size <= 0 || packet_loss_percentage < 0 || packet_loss_percentage > 100) {
        printf("Invalid arguments: port and buffer_size must be positive, packet_loss between 0-100\n");
        exit(1);
    }

    // Размер пакета с учетом гибкого массива данных
    size_t packet_size = sizeof(tcp_packet_t) + buf_size;
    tcp_packet_t *packet = malloc(packet_size);
    tcp_packet_t *syn_ack = malloc(packet_size);
    tcp_packet_t *ack_packet = malloc(packet_size);
    tcp_packet_t *fin_ack = malloc(packet_size);
    
    if (!packet || !syn_ack || !ack_packet || !fin_ack) {
        perror("malloc");
        free(packet); free(syn_ack); free(ack_packet); free(fin_ack);
        exit(1);
    }

    int sockfd, n;
    char ipadr[16];
    struct sockaddr_in servaddr;
    
    // Хранение состояния соединения
    uint32_t expected_seq = 0;
    uint32_t next_seq = 1;
    int connection_established = 0;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket problem");
        free(packet); free(syn_ack); free(ack_packet); free(fin_ack);
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(serv_port);

    if (bind(sockfd, (SADDR *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind problem");
        free(packet); free(syn_ack); free(ack_packet); free(fin_ack);
        close(sockfd);
        exit(1);
    }
    
    printf("SERVER starts on port %d with buffer_size=%d, packet_loss=%d%%...\n", 
           serv_port, buf_size, packet_loss_percentage);

    srand(time(NULL));

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        n = recvfrom(sockfd, packet, packet_size, 0, 
                    (SADDR *)&client_addr, &client_len);
        
        if (n < 0) {
            perror("recvfrom");
            continue;
        }

        // Имитация потери пакетов
        if (rand() % 100 < packet_loss_percentage) {
            printf(">>> Simulating packet loss <<<\n");
            continue;
        }

        // Преобразование из сетевого порядка байт
        packet->seq_num = ntohl(packet->seq_num);
        packet->ack_num = ntohl(packet->ack_num);
        packet->flags = ntohs(packet->flags);
        packet->data_len = ntohl(packet->data_len);

        printf("Received packet: seq=%u, ack=%u, flags=0x%x, len=%u\n", 
               packet->seq_num, packet->ack_num, packet->flags, packet->data_len);

        // Обработка SYN (установка соединения)
        if (packet->flags & SYN_FLAG) {
            printf("SYN received, establishing connection...\n");
            connection_established = 1;
            expected_seq = packet->seq_num + 1;
            
            // Отправка SYN-ACK
            memset(syn_ack, 0, packet_size);
            syn_ack->seq_num = htonl(next_seq);
            syn_ack->ack_num = htonl(expected_seq);
            syn_ack->flags = htons(SYN_FLAG | ACK_FLAG);
            syn_ack->data_len = 0;
            
            sendto(sockfd, syn_ack, packet_size, 0,
                  (SADDR *)&client_addr, client_len);
            next_seq++;
            continue;
        }

        // Обработка FIN (завершение соединения)
        if (packet->flags & FIN_FLAG) {
            printf("FIN received, closing connection...\n");
            connection_established = 0;
            expected_seq = 0;
            next_seq = 1;
            
            // Отправка ACK на FIN
            memset(fin_ack, 0, packet_size);
            fin_ack->seq_num = htonl(next_seq);
            fin_ack->ack_num = htonl(packet->seq_num + 1);
            fin_ack->flags = htons(ACK_FLAG);
            fin_ack->data_len = 0;
            
            sendto(sockfd, fin_ack, packet_size, 0,
                  (SADDR *)&client_addr, client_len);
            continue;
        }

        // Проверка порядкового номера
        if (connection_established && packet->seq_num != expected_seq) {
            printf("Wrong sequence number: expected %u, got %u\n", 
                   expected_seq, packet->seq_num);
            continue;
        }

        // Обработка данных
        if (packet->data_len > 0 && connection_established) {
            if (packet->data_len < buf_size) {
                packet->data[packet->data_len] = 0; // завершающий ноль
            }
            
            printf("REQUEST: %s FROM %s:%d\n", packet->data,
                   inet_ntop(AF_INET, &client_addr.sin_addr, ipadr, 16),
                   ntohs(client_addr.sin_port));

            // Подготовка ответа
            memset(ack_packet, 0, packet_size);
            ack_packet->seq_num = htonl(next_seq);
            ack_packet->ack_num = htonl(expected_seq + packet->data_len);
            ack_packet->flags = htons(ACK_FLAG);
            
            // Копируем данные обратно
            size_t copy_size = (packet->data_len < buf_size) ? packet->data_len : buf_size;
            memcpy(ack_packet->data, packet->data, copy_size);
            ack_packet->data_len = htonl(copy_size);
            
            sendto(sockfd, ack_packet, packet_size, 0,
                  (SADDR *)&client_addr, client_len);
            
            expected_seq += packet->data_len;
            next_seq++;
        }
    }
    
    free(packet); free(syn_ack); free(ack_packet); free(fin_ack);
    close(sockfd);
    return 0;
}