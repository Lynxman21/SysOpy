#include<stdio.h>
#include<time.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<stdlib.h>

double function(double x) {
    return 4/(x*x + 1);
}

double integral(double (*func)(double),double start,double end,double dx) {
    double res = 0.0;
    double x = start;
    int steps = (int)((end-start)/dx);

    for (int i=0;i<steps;i++) {
        res += dx*func(x);
        x += dx;
    }

    double reminder = end-x;
    if (reminder > 0.0) {
        res += reminder*func(x);
    }

    return res;
}

void counter(double dx, int k) {
    clock_t start = clock();
    double range = 1.0/k;
    double ans = 0;

    for (int i=0;i<k;i++) {
        int fd[2];
        pipe(fd);
        pid_t process = fork();

        if (process == 0) {
            close(fd[0]);
            double partial_res = integral(function,i*range,i*range+range,dx);
            write(fd[1],&partial_res,sizeof(double));
            close(fd[1]);
            exit(0);
        }
        else {
            close(fd[1]);
            double partial_res;
            read(fd[0],&partial_res,sizeof(double));
            close(fd[0]);
            ans += partial_res;
        }
    }

    for (int i =0; i<k;i++) wait(NULL);

    clock_t end = clock();
    double time_spent = (double)(end-start)/CLOCKS_PER_SEC;
    printf("Proces liczenia dla k=%d zakończył się wynikiem %f w czasie %f\n",k,ans,time_spent);
    printf("\n");
}

int main(int argc,char* argv[]) {
    if (argc != 3) {
        printf("Zła ilość argumentów");
        return 1;
    }

    double dx = strtod(argv[1], NULL);
    int k = atoi(argv[2]);

    for (int i=1;i<=k;i++) counter(dx,i);

    return 0;
}