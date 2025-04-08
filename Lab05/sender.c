#include<stdio.h>
#include<sys/types.h>
#include<signal.h>

void handler() {
    printf("Potwiedzenie odebrania sygnału. Koniec działania\n");
}

int main(int argc,char* argv[]) {
    if (argc != 3) {
        printf("Zła ilość argumentów");
        return 1;
    }

    int catcher_pid;
    int mode;
    
    if (sscanf(argv[1],"%d",&catcher_pid)!=1) {
        printf("Nie podano pid\n");
        return 1;
    }
    if (sscanf(argv[2],"%d",&mode)!=1) {
        printf("Nie podano liczby\n");
        return 1;
    }
    if (mode > 5 || mode < 1) {
        printf("Zły tryb pracy\n");
        return 1;
    }

    union sigval val;
    val.sival_int = mode;

    sigqueue((pid_t) catcher_pid, SIGUSR1,val);
    signal(SIGUSR1,handler);
    pause();
    
    return 0;
}