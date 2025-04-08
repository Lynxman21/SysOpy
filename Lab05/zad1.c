#include<stdio.h>
#include<string.h>
#include<signal.h>

void hand(int sig_no) {
    (void)sig_no;
    printf("Otrzymano sygnał SIGUSR1\n");
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Zła ilość argumentów\n");
        return 1;
    }

    if (strcmp(argv[1],"none")==0) {
        signal(SIGUSR1,SIG_DFL);
    }
    if (strcmp(argv[1],"ignore")==0) {
        signal(SIGUSR1,SIG_IGN);
    }
    if (strcmp(argv[1],"handler")==0) {
        signal(SIGUSR1,hand);
    }
    if (strcmp(argv[1],"mask")==0) {
        sigset_t newmask;

        sigemptyset(&newmask);
        sigaddset(&newmask, SIGUSR1);
        if (sigprocmask(SIG_SETMASK, &newmask, NULL) < 0) perror("Błąd przy ustawianiu maski sygnałów\n");
    }

    raise(SIGUSR1);

    if (strcmp(argv[1],"mask")==0) {
        sigset_t pending;
        sigpending(&pending);
        if (sigismember(&pending,SIGUSR1)) {
            printf("SIGUSR1 jest sygnałem oczekującym\n");
        } else {
            printf("SIGUSR1 nie jest sygnałem oczekującym\n");
        }
    }
    return 0;
}