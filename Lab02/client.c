#include <stdio.h>

#ifdef D
#include <dlfcn.h>
#endif

#ifndef D
#include "collatz.h"
#endif

#define MAX_ITER 100

void main() {

    int vals[10] = {20,1,4,32,7,100,16,3,28,60};
    int steps[MAX_ITER];

    #ifdef D
    void *collatz = dlopen("libcollatz.so",RTLD_LAZY);
    if (!collatz) {
        printf("%s",dlerror());
        return 1;
    }
    
    int (*test_collatz_convergance)(int,int,int*);
    test_collatz_convergance = (int (*)(int,int,int*))dlsym(collatz,"test_collatz_convergance");

    if (dlerror() != NULL) {
        printf("%s",dlerror());
        dlclose(collatz);
        return 1;
    } 

    for (int i = 0;i<10;i++) {
        int how_long = test_collatz_convergance(vals[i],MAX_ITER,steps);

        if (how_long > 0) {
            printf("Input zbiega do 1 po %d wykonaniach\n",how_long);
            for (int j=0;j<how_long;j++) {
                printf("%d -> ",steps[j]);
            }
            printf("\n");
        }
        else {
            printf("Wartość nie zbiega do 1 w %d iteracjach\n",MAX_ITER);
        }
    }

    dlclose(collatz);
    #endif

    
    #ifndef D
    for (int i = 0;i<10;i++) {
        int how_long = test_collatz_convergance(vals[i],MAX_ITER,steps);

        if (how_long > 0) {
            printf("Input zbiega do 1 po %d wykonaniach\n",how_long);
            for (int j=0;j<how_long;j++) {
                printf("%d ",steps[j]);
                if (j!=how_long-1) {
                    printf("-> ");
                }
            }
            printf("\n");
        }
        else {
            printf("Wartość nie zbiega do 1 w %d iteracjach\n",MAX_ITER);
        }
    }
    #endif

    return 1;
}