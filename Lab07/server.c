#include <stdio.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MSG_SIZE 1024

//Kolejka typu POSIX

typedef struct {
    int id;
    mqd_t queue;
} client_t;

void send_to_all(client_t *, int, size_t, char *);
int delete_client(client_t *, int, size_t);

int main() {
    size_t len = 1; // Actuall size of array
    int current_clients = 0; // Number of clients
    
    client_t *clients = malloc(len * sizeof(client_t));
    mqd_t server_queue;
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;

    if ((server_queue = mq_open("/server_queue",O_CREAT | O_RDONLY,0666,&attr)) == -1) {
        perror("Open server queue failed");
        return 1;
    }

    printf("Server queue is working\n");
    char buffer[MAX_MSG_SIZE+1];

    while(1) {
        ssize_t msq_size = mq_receive(server_queue, buffer, MAX_MSG_SIZE, NULL);

        if (msq_size == -1) {
            perror("Receive failed");
            break;
        }
        
        if (strncmp(buffer, "INIT", 4) == 0) {
            char client_path[MAX_MSG_SIZE];
            sscanf(buffer+5, "%s",client_path);
            mqd_t client_queue = mq_open(client_path, O_WRONLY);

            if (client_queue == -1) {
                perror("Open client queue failed");
                continue;
            }

            if (current_clients == len) {
                len *= 2;
                client_t *temp = realloc(clients, len*sizeof(client_t));
                if (temp == NULL) {
                    perror("Realloc failed");
                    free(clients);
                    return 1;
                }
                clients = temp;
            }
            clients[current_clients].id = current_clients;
            clients[current_clients].queue = client_queue;
            current_clients++;
            
            snprintf(buffer,MAX_MSG_SIZE,"%d",clients[current_clients-1].id);
            mq_send(client_queue, buffer, strlen(buffer)+1, 0);
            printf("Client %d connected\n", clients[current_clients-1].id);
        } else if (strncmp(buffer, "SEND", 4) == 0) {
            int id;
            char msg[MAX_MSG_SIZE] = {0};
            sscanf(buffer+5, "%d %[^\n]",&id,msg);
            send_to_all(clients, id, current_clients, msg);
        } else {
            int id;
            sscanf(buffer+5, "%d",&id);
            printf("Client %d disconnected\n", id);

            if (delete_client(clients, id, current_clients) == 0) {
                current_clients--;
            }

            if (current_clients == 0) {
                break;
            }
        }
    }

    free(clients);
    mq_close(server_queue);
    mq_unlink("/server_queue");
    return 0;
}

void send_to_all(client_t *clients, int id, size_t len, char *msg) {
    for (int i = 0; i < len; i++) {
        if (clients[i].id != id) {
            mq_send(clients[i].queue, msg, strlen(msg)+1, 0);
        }
    }
}

int delete_client(client_t *clients, int id, size_t len) {
    int index = -1;
    for (int i = 0; i < len; i++) {
        if (clients[i].id == id) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        return -1;
    }

    char client_path[MAX_MSG_SIZE];
    sprintf(client_path, "/%d",clients[index].id);

    mq_send(clients[index].queue, "EXIT", strlen("EXIT")+1, 0);
    mq_close(clients[index].queue);
    for (int i = index; i < len-1; i++) {
        clients[i] = clients[i+1];
    }
    return 0;
}