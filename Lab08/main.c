#include <stdio.h>
// shared memory
#include <fcntl.h> //shm_open
#include <sys/stat.h> // do trybów dostępu (np. S_IRUSR | S_IWUSR)
#include <sys/mman.h> //mmap(), shm_open(), shm_unlink()
// semaphores
#include <semaphore.h> //do sem_open(), sem_wait(), sem_post(), sem_close(), sem_unlink()
//some other functions
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>


#define QUEUE_SIZE 10

volatile int running = 1;

void sigint_handler(int signum) {
    running = 0;
}

typedef struct {
    char tasks[QUEUE_SIZE][11];
    int head;
    int tail;
} to_print_queue;

void generate_msg(char *msg, int length) {
    for (int i=0;i<length;i++) {
        msg[i] = 'a' + rand() % 26;
    }
    msg[length] = '\0';
}

void printer_process(int index,to_print_queue *queue, sem_t *sem_client, sem_t *sem_printer, sem_t *sem_queue, sem_t *sem_output){
    while (running) {
        sem_wait(sem_printer);
        sem_wait(sem_queue);
        
        char msg[11];
        strcpy(msg, queue->tasks[queue->head]);
        queue->head = (queue->head + 1) % QUEUE_SIZE;
        sem_post(sem_queue);
        sem_post(sem_client);

        sem_wait(sem_output);
        printf("Drukarka %d\n",index);

        for (int i=0;i<10;i++) {
            printf("%c", msg[i]);
            fflush(stdout);
            sleep(1);
        }

        printf("\n");
        sem_post(sem_output);
    }
}

void client_process(to_print_queue *queue, sem_t *sem_client, sem_t *sem_printer, sem_t *sem_queue){
    while (running) {
        char msg[11];
        generate_msg(msg, 10);
        sem_wait(sem_client);
        sem_wait(sem_queue);
        strcpy(queue->tasks[queue->tail], msg);
        queue->tail = (queue->tail + 1) % QUEUE_SIZE;
        sem_post(sem_queue);
        sem_post(sem_printer);
        sleep(1 + rand() % 5);
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        perror("arguments");
        return 1;
    }

    signal(SIGINT, sigint_handler);

    int N = atoi(argv[1]);
    int M = atoi(argv[2]);

    sem_t *sem_printer = sem_open("/sem_printer", O_CREAT | O_EXCL, 0666, 0);
    sem_t *sem_client = sem_open("/sem_client", O_CREAT | O_EXCL, 0666, QUEUE_SIZE);
    sem_t *sem_queue = sem_open("/sem_queue", O_CREAT | O_EXCL, 0666, 1);
    sem_t *sem_output = sem_open("/sem_output", O_CREAT | O_EXCL, 0666, 1);

    if (sem_printer == SEM_FAILED || sem_client == SEM_FAILED || sem_queue == SEM_FAILED || sem_output == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    int fd = shm_open("/printer", O_CREAT | O_EXCL | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        return 1;
    }

    ftruncate(fd, sizeof(to_print_queue));
    to_print_queue *queue = mmap(NULL, sizeof(to_print_queue), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (queue == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    queue->head = 0;
    queue->tail = 0;

    for (int i=0;i<N+M;i++) {
        pid_t pid = fork();

        if (pid == 0) {
            if (i < N) {
                client_process(queue, sem_client, sem_printer, sem_queue);
            } else {
                int index = i - N;
                printer_process(index,queue, sem_client, sem_printer, sem_queue, sem_output);
            }
            exit(0);
        }
    }

    for (int i=0;i<N+M;i++) {
        wait(NULL);
    }
    sem_close(sem_printer);
    sem_close(sem_client);
    sem_close(sem_queue);
    sem_close(sem_output);
    sem_unlink("/sem_printer");
    sem_unlink("/sem_client");
    sem_unlink("/sem_queue");
    sem_unlink("/sem_output");
    munmap(queue, sizeof(to_print_queue));
    shm_unlink("/printer");
    close(fd);

    return 0;
}