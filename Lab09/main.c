#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

typedef struct {
    double start;
    double end;
    double dx;
    double res;
} thread_data_t;

double f(double x) {
    return 4/(x*x + 1);
}

void* integral(void *args) {
    thread_data_t *data = (thread_data_t *)args;
    if (data->dx <= 0 || data->start > data->end) return 0;

    double sum = 0;
    double x;
    double start = data->start;
    while (start+data->dx <= data->end) {
        x = start + data->dx/2;
        sum += f(x) * data->dx;
        start += data->dx;
    }

    if (start < data->end) {
        x = (start + data->end)/2;
        sum += f(x) * (data->end - start);
    }

    data->res = sum;
    return NULL;
}

int main(int argc, char **argv) {
    double dx = atof(argv[1]);
    int n = atoi(argv[2]);

    if (dx <= 0 || n <= 0) {
        fprintf(stderr, "Invalid input\n");
        return 1;
    }

    struct timespec start, end;
    double time, total;
    pthread_t threads[n];
    thread_data_t data[n];

    for (int i=1;i<=n;i++) {
        total = 0.0;
        double interval = 1.0/i;

        clock_gettime(CLOCK_MONOTONIC, &start);

        for (int j=0;j<i;j++) {
            data[j].start = j * interval;
            data[j].end = (j+1) * interval;
            data[j].dx = dx;
            data[j].res = 0.0;
            pthread_create(&threads[j],NULL,integral,&data[j]);
        }

        for (int j=0;j<i;j++) {
            pthread_join(threads[j],NULL);
            total += data[j].res;
        }

        clock_gettime(CLOCK_MONOTONIC, &end);
        time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1e9;

        printf("Wynik dla %d wątków wynosi %f i został obliczony w czasie %f\n", i, total, time);
    }

    return 0;
}