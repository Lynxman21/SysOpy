#include <stdio.h>
#include <fcntl.h>
#include <mqueue.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_MSG_SIZE 1024

int main() {
    int id = getpid();
    char client_queue_name[100];
    sprintf(client_queue_name, "/%d",id);

    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;

    mqd_t server_queue = mq_open("/server_queue", O_WRONLY);
    mqd_t client_queue = mq_open(client_queue_name, O_CREAT | O_RDONLY, 0644, &attr);
    if (server_queue == -1) {
        perror("Open server queue failed");
        return 1;
    }
    if (client_queue == -1) {
        perror("Open client queue failed");
        return 1;
    }

    char msg[MAX_MSG_SIZE] = {0};
    sprintf(msg, "INIT /%d", id);
    mq_send(server_queue, msg, strlen(msg)+1, 0);
    memset(msg, 0, sizeof(msg));
    mq_receive(client_queue, msg, MAX_MSG_SIZE, NULL);
    
    int client_id = atoi(msg);
    printf("Client %d connected\n", client_id);

    pid_t pid = fork();

    if (pid == 0) {
        while(1) {
            printf("Enter message (SEND id msg/EXIT id): ");
            char msg[MAX_MSG_SIZE] = {0};
            fgets(msg,MAX_MSG_SIZE,stdin);
            mq_send(server_queue, msg, strlen(msg)+1, 0);

            if (strncmp(msg, "EXIT", 4) == 0) { //Można było sygnałem wsm zakończyć działanie 
                break;
            }

            if (strncmp(msg, "SEND", 4) == 0) {
                continue;
            } else {
                printf("Invalid command\n");
            }
        }
        exit(0);
    } else {
        while(1) {
            char msg[MAX_MSG_SIZE] = {0};
            mq_receive(client_queue,msg,MAX_MSG_SIZE,NULL);
            if (strncmp(msg, "EXIT", 4) == 0) {
                break;
            }
            printf("\nReceived message: %s\n", msg);
            printf("Enter message (SEND id msg/EXIT id): ");
        }
    }

    wait(NULL);
    mq_close(client_queue);
    mq_close(server_queue);
    mq_unlink(client_queue_name);
    return 0;
}