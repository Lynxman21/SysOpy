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

void handle_ctrl_c(int sig) {
    flag = 1;
    send(socket_fd, "STOP",4,0);
    close(socket_fd);
    exit(0);
}

void *receive_mess(void *arg) {
    char buffer[1024];
    while(!flag) {
        ssize_t bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
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
            send(socket_fd, "WORKING", 7, 0);
            continue;
        }
        printf("Otrzymano: %s", buffer);
    }
}

void *send_mess(void *arg) {
    char message[1024];
    while(fgets(message,1024,stdin) && !flag) {
        send(socket_fd, message, strlen(message), 0);
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

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    signal(SIGINT, handle_ctrl_c);
    
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    inet_pton(AF_INET,ip,&server_address.sin_addr);

    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Błąd połączenia z serwerem");
        close(socket_fd);
        return 1;
    }

    send(socket_fd, name, strlen(name)+1, 0);

    pthread_t receive_thread, send_thread;
    pthread_create(&receive_thread, NULL, receive_mess, NULL);
    pthread_create(&send_thread, NULL, send_mess, NULL);

    pthread_join(receive_thread, NULL);
    pthread_join(send_thread, NULL);

    close(socket_fd);
    return 0;
}