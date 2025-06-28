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
    struct sockaddr_in addr;
    char name[NAME_MAX_LENGTH];
    int alive;
} client_t;

typedef struct {
    char mess[BUFFER_SIZE];
    struct sockaddr_in addr;
    socklen_t addr_len;
} msg_t;

client_t clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int server_fd;

void get_current_time(char *buffer, size_t size) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer,size,"%Y-%m-%d %H:%M:%S",tm_info);
}

void *ping_all(void *arg) {
    int sockfd = *(int *)arg;

    while (1) {
        sleep(10);
        pthread_mutex_lock(&clients_mutex);
        for (int i=0;i<MAX_CLIENTS;i++) {
            if (clients[i].alive) {
                ssize_t sent = sendto(sockfd,"ALIVE",5,0,(struct sockaddr *)&clients[i].addr, sizeof(clients[i].addr));
                if (sent <= 0) {
                    printf("Klient %s nie odpowiadła. Zatem zostaje usunięty z listy\n", clients[i].name);
                    clients[i].alive = 0;
                    clients[i].name[0] = '\0';
                    memset(&clients[i].addr,0,sizeof(clients[i].addr));
                }
            }
        }

        pthread_mutex_unlock(&clients_mutex);
    }
    return NULL;
}

void *client_mess(void *arg) {
    msg_t *msg = (msg_t *)arg;
    char *buffer = msg->mess;
    struct sockaddr_in client_addr = msg->addr;
    socklen_t addr_len = msg->addr_len;

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

    if (strncmp(buffer,"NAME ",5) == 0) {
        char *name = buffer + 5;

        pthread_mutex_lock(&clients_mutex);

        int inserted = 0;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i].alive) {
                clients[i].addr = client_addr;
                strncpy(clients[i].name,name,NAME_MAX_LENGTH);
                clients[i].alive = 1;
                inserted = 1;
                break;
            }
        }

        pthread_mutex_unlock(&clients_mutex);

        if (!inserted) {
            char *msg = "Serwer pełny\n";
            sendto(server_fd,msg,strlen(msg),0,(struct sockaddr *)&client_addr,addr_len);
        }
    } else if (strncmp(buffer, "LIST",4) == 0) {
        char list[BUFFER_SIZE] = "Aktywni klienci:\n";
        pthread_mutex_lock(&clients_mutex);
        for (int i=0;i<MAX_CLIENTS;i++) {
            if (clients[i].alive) {
                strcat(list, clients[i].name);
                strcat(list,"\n");
            }
        }

        pthread_mutex_unlock(&clients_mutex);
        sendto(server_fd,list,strlen(list),0,(struct sockaddr *)&client_addr,addr_len);
    } else if (strncmp(buffer,"2ALL ",5) == 0) {
        char sender[NAME_MAX_LENGTH] = "Anonim";
        pthread_mutex_lock(&clients_mutex);

        for (int i=0;i<MAX_CLIENTS;i++) {
            if (clients[i].alive) {
                strcpy(sender,clients[i].name);
                break;
            }
        }

        pthread_mutex_unlock(&clients_mutex);

        char stime[64];
        get_current_time(stime,sizeof(stime));
        char msg_out[BUFFER_SIZE];
        snprintf(msg_out,sizeof(msg_out),"[%s] %s: %s\n",stime,sender, buffer + 5);

        pthread_mutex_lock(&clients_mutex);

        for (int i=0;i<MAX_CLIENTS;i++) {
            if (clients[i].alive) {
                sendto(server_fd,msg_out,strlen(msg_out),0,(struct sockaddr*)&clients[i].addr, sizeof(clients[i].addr));
            }
        }

        pthread_mutex_unlock(&clients_mutex);
    } else if (strncmp(buffer,"2ONE ",5) == 0) {
        char *target = strtok(buffer + 5, " ");
        char *msg_content = strtok(NULL,"\n");

        if (target && msg_content) {
            char sender[NAME_MAX_LENGTH] = "Anonim";

            pthread_mutex_lock(&clients_mutex);

            for (int i=0;i<MAX_CLIENTS;i++) {
                if (clients[i].alive && clients[i].addr.sin_port == client_addr.sin_port && clients[i].addr.sin_addr.s_addr == client_addr.sin_addr.s_addr) {
                    strcpy(sender,clients[i].name);
                    break;
                }
            }

            pthread_mutex_unlock(&clients_mutex);

            char timestr[64];
            get_current_time(timestr,sizeof(timestr));
            char msg_out[BUFFER_SIZE];
            snprintf(msg_out, sizeof(msg_out), "[%s] %s: %s\n",timestr,sender,msg_content);
                
            pthread_mutex_lock(&clients_mutex);

            for (int i=0;i<MAX_CLIENTS;i++) {
                if (clients[i].alive && strcmp(clients[i].name, target) == 0) {
                    sendto(server_fd,msg_out,strlen(msg_out),0,(struct sockaddr *)&clients[i].addr, sizeof(clients[i].addr));
                    break;
                }
            }

        pthread_mutex_unlock(&clients_mutex);
        }
    } else if (strncmp(buffer,"STOP",4) == 0) {
        pthread_mutex_lock(&clients_mutex);

        for (int i = 0;i<MAX_CLIENTS;i++) {
            if (clients[i].alive && clients[i].addr.sin_port == client_addr.sin_port && clients[i].addr.sin_addr.s_addr == client_addr.sin_addr.s_addr) {
                clients[i].alive = 0;
                clients[i].name[0] = '\0';
                memset(&clients[i].addr,0,sizeof(clients[i].addr));
                break;
            }
        }

        pthread_mutex_unlock(&clients_mutex);
    }
    free(msg);
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

    server_fd = socket(AF_INET,SOCK_DGRAM,0);
    signal(SIGINT,exit_handler);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET,ip,&server_addr.sin_addr);

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    printf("Server nasłuchuje na porcie %d\n",port);

    pthread_t ping_pth;
    pthread_create(&ping_pth,NULL,ping_all,&server_fd);

    while (1) {
        msg_t *msg = malloc(sizeof(msg_t));
        msg->addr_len = sizeof(msg->addr);
        ssize_t recieved = recvfrom(server_fd,msg->mess,BUFFER_SIZE-1,0,(struct sockaddr *)&msg->addr, &msg->addr_len);

        if (recieved == 0) {
            free(msg);
            continue;
        }

        msg->mess[recieved] = '\0';
        pthread_t client_pth;
        pthread_create(&client_pth,NULL,client_mess,msg);
    }

    close(server_fd);
    return 0;
}