#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

#define NAME_MAX_LENGTH 100
#define MAX_CLIENTS 15
#define BUFFER_SIZE 1024

typedef struct {
    int socket_fd;
    char name[NAME_MAX_LENGTH];
    int alive;
} client_t;

client_t clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int server_fd;

void remove_client(int fd) {
    pthread_mutex_lock(&clients_mutex);

    for (int i=0;i<MAX_CLIENTS;i++) {
        if (clients[i].alive && clients[i].socket_fd == fd) {
            clients[i].alive = 0;
            close(fd);
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void get_current_time(char *buffer, size_t size) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer,size,"%Y-%m-%d %H:%M:%S",tm_info);
}

void *ping_all(void *arg) {
    while (1) {
        sleep(10);
        pthread_mutex_lock(&clients_mutex);
        for (int i=0;i<MAX_CLIENTS;i++) {
            if (clients[i].alive) {
                ssize_t sent = send(clients[i].socket_fd,"ALIVE",5,0);
                if (sent <= 0) {
                    printf("Klient %s nie odpowiadła. Zatem zostaje usunięty z listy\n", clients[i].name);
                    close(clients[i].socket_fd);
                    clients[i].socket_fd = 0;
                    clients[i].alive = 0;
                    clients[i].name[0] = '\0';
                }
            }
        }

        pthread_mutex_unlock(&clients_mutex);
    }
    return NULL;
}

void *client_mess(void *arg) {
    int sock_fd = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    char name[NAME_MAX_LENGTH];

    recv(sock_fd, name, NAME_MAX_LENGTH-1, 0);

    pthread_mutex_lock(&clients_mutex);

    int index = -1;

    for (int i=0;i<MAX_CLIENTS;i++) {
        if (!clients[i].alive) {
            clients[i].socket_fd = sock_fd;
            strcpy(clients[i].name,name);
            clients[i].alive = 1;
            index = i;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    if (index == -1) {
        char *msg = "Serwer pełny\n";
        send(sock_fd,msg,strlen(msg),0);
        close(sock_fd);
        return;
    }

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int recived = recv(sock_fd, buffer, BUFFER_SIZE-1, 0);

        if (recived <= 0) {
            printf("Klient %s się rozłączył.\n",name);
            break;
        }

        buffer[recived] = '\0';

        if (strncmp(buffer, "LIST", 4) == 0) {
            char list[BUFFER_SIZE] = "Aktywni klienci:\n";
            pthread_mutex_lock(&clients_mutex);
            for (int i=0;i<MAX_CLIENTS;i++) {
                if (clients[i].alive) {
                    strcat(list, clients[i].name);
                    strcat(list,"\n");
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            send(sock_fd,list,strlen(list),0);
        } else if (strncmp(buffer, "2ALL ", 5) == 0) {
            char timestr[64];
            get_current_time(timestr,sizeof(timestr));
            char msg[BUFFER_SIZE];
            snprintf(msg,sizeof(msg),"[%s] %s: %s\n",timestr, name, buffer + 5);
            
            pthread_mutex_lock(&clients_mutex);

            for (int i=0;i<MAX_CLIENTS;i++) {
                if (clients[i].alive && clients[i].socket_fd != sock_fd) {
                    send(clients[i].socket_fd, msg, strlen(msg),0);
                }
            }

            pthread_mutex_unlock(&clients_mutex);


        } else if (strncmp(buffer, "2ONE ", 5) == 0) {
            char *target = strtok(buffer + 5, " ");
            char *msg_content = strtok(NULL,"\n");

            if (target && msg_content) {
                char timestr[64];
                get_current_time(timestr,sizeof(timestr));
                char msg[BUFFER_SIZE];
                snprintf(msg, sizeof(msg), "[%s] %s: %s\n",timestr,name,msg_content);
                
                pthread_mutex_lock(&clients_mutex);

                for (int i=0;i<MAX_CLIENTS;i++) {
                    if (clients[i].alive && strcmp(clients[i].name, target) == 0) {
                        send(clients[i].socket_fd,msg,strlen(msg),0);
                        break;
                    }
                }

                pthread_mutex_unlock(&clients_mutex);
            }
        } else if (strncmp(buffer, "STOP", 4) == 0) {
            break;
        } else if (strncmp(buffer, "WORKING", 7) == 0) {
            continue;
        }
    }
    remove_client(sock_fd);
    return NULL;
}

void exit_handler(int sig) {
    close(server_fd);
    exit(0);
}

int main(int argc, char **argv) {
    if(argc < 3) {
        printf("Za mało argumentów.\n");
        return 1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);

    server_fd = socket(AF_INET,SOCK_STREAM,0);
    signal(SIGINT,exit_handler);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET,ip,&server_addr.sin_addr);

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, MAX_CLIENTS);

    printf("Server nasłuchuje na porcie %d\n",port);

    pthread_t ping_thread;
    pthread_create(&ping_thread,NULL,ping_all,NULL);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int *client_sock = malloc(sizeof(int));
        *client_sock = accept(server_fd,(struct sockaddr *)&client_addr, &len);
        pthread_t client_pth;
        pthread_create(&client_pth,NULL,client_mess,client_sock);
    }

    close(server_fd);
    return 0;
}