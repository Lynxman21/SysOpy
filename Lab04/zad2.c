#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

int global = 10;

int main(int argc, char *argv[]) {
    int local = 20;

    if (argc != 2) {
        printf("Zła liczna argumentów");
        return 1;
    }

    printf("Nazwa programu to %s\n",argv[0]);

    pid_t pid = fork();

    if (pid == -1) {
        printf("Błąd w tworzeniu procesu");
        return 1;
    } else if (pid == 0) {
        printf("Child process\n");
        global++;
        local++;
        printf("child's pid = %d, parent pid = %d\n",getpid(), getppid());
        printf("child's local = %d, child's global = %d\n",local,global);
        execl("/bin/ls","ls",argv[1],NULL);
    } else {
        int exit_code;
        printf("Parent process\n");
        printf("parent pid = %d, child's pid = %d\n",getppid(),getpid());
        wait(&exit_code);
        printf("Child exit code: %d\n",exit_code);
        printf("Parent's local = %d, Parent's global = %d\n",local,global);
    }

    return 0;
}