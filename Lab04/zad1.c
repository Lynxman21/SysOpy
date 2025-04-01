#include <stdio.h>
#include <sys/types.h> //typy
#include <stdlib.h> //atoi
#include <unistd.h> //get
#include <sys/wait.h> //wait

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Zła ilość argumentów!");
        return 1;
    }

    int how_many = atoi(argv[1]); //wskaźnik na string i mam inta

    for (int i=0;i<how_many;i++) {
        pid_t child_id = fork();

        if (child_id == 0) {
            printf("Parent is: %d\n",getppid());
            printf("Child is: %d\n",getpid());
            return 0;
        }
    }

    for (int i=0;i<how_many;i++) {
        wait(NULL);
    }

    return 0;
}