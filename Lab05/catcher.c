#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<signal.h>

int change_mode_counter = 0;
int current_mode = 0;

void handler2(int sig_no) {
    (void)sig_no;
    const char msg[] = "Wciśnięto CTRL+C\n";
    write(STDOUT_FILENO, msg, sizeof(msg)-1);
}

void handler(int sig_no, siginfo_t *info, void *ucontext) {
    change_mode_counter++;
    kill(info->si_pid,SIGUSR1);

    switch (info->si_value.sival_int)
    {
    case 1:
        printf("Ilość wywołań: %d\n",change_mode_counter);
        current_mode = 1;
        break;
    case 2:
        current_mode = 2;
        break;
    case 3:
        current_mode = 3;
        signal(SIGINT, SIG_IGN);
        break;
    case 4:
        current_mode = 4;
        signal(SIGINT,handler2);
        break;
    case 5:
        current_mode = 5;
        break;
        
    default:
        break;
    }
}

int val_per_sec() {
    int counter = 0;
    while (current_mode==2) {
        printf("%d\n",counter);
        counter++;
        sleep(1);
    }
}

int main() {
    pid_t c_pid = getpid();
    printf("PID catcher'a: %d\n",(int)c_pid);

    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags = SA_SIGINFO;

    sigaction(SIGUSR1,&sa,NULL);

    while (1) {
        if (current_mode == 2) {
            val_per_sec();
        } else if (current_mode == 5) {
            return 0;
        }
        pause();
    }
}