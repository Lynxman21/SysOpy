#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>

#define MAX_NAME_LENGTH 100

int socket_fd;
int flag = 0;
struct sockaddr_in server_address;
socklen_t server_addr_len;

void handle_ctrl_c(int sig) {
    flag = 1;
    sendto(socket_fd, "STOP",5,0, (struct sockaddr *)&server_address, server_addr_len);
    close(socket_fd);
    exit(0);
}

void *receive_mess(void *arg) {
    char buffer[1024];
    while(!flag) {
        ssize_t bytes_received = recvfrom(socket_fd, buffer, sizeof(buffer) - 1, 0,NULL,NULL);
        if (bytes_received < 0) {
            perror("Błąd odbioru wiadomości lub połączenie zamknięte");
            close(socket_fd);
            exit(1);
        } else if (bytes_received == 0) {
            printf("Połączenie zamknięte przez serwer.\n");
            close(socket_fd);
            exit(0);
        }

        buffer[bytes_received] = '\0';
        if (strncmp(buffer, "ALIVE",5) == 0) {
            sendto(socket_fd, "WORKING", 7, 0,(struct sockaddr *)&server_address, server_addr_len);
            continue;
        }
        printf("Otrzymano: %s", buffer);
    }
}

void *send_mess(void *arg) {
    char message[1024];
    while(fgets(message,1024,stdin) && !flag) {
        sendto(socket_fd, message, strlen(message), 0, (struct sockaddr *)&server_address, server_addr_len);
    }
}

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("Za mało argumentów.\n ");
        return 1;
    }

    char name[MAX_NAME_LENGTH];
    strcpy(name,argv[1]);
    char *ip = argv[2];
    int port = atoi(argv[3]);

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    signal(SIGINT, handle_ctrl_c);
    
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    inet_pton(AF_INET,ip,&server_address.sin_addr);
    server_addr_len = sizeof(server_address);

    char name_cmd[1024];
    snprintf(name_cmd,1024,"NAME %s",name);

    sendto(socket_fd, name_cmd, strlen(name_cmd)+1, 0,(struct sockaddr *)&server_address, server_addr_len);

    pthread_t receive_thread, send_thread;
    pthread_create(&receive_thread, NULL, receive_mess, NULL);
    pthread_create(&send_thread, NULL, send_mess, NULL);

    pthread_join(receive_thread, NULL);
    pthread_join(send_thread, NULL);

    close(socket_fd);
    return 0;
}