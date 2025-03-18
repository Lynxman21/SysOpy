#include "collatz.h"
#include <stdio.h>

int collatz_conjecture(int input) {
    if (input%2 == 0) {
        return input/2;
    }
    else {
        return 3*input + 1;
    }
}

int test_collatz_convergance(int input, int max_iter, int *steps) {
    int current_val = input;

    if (input == 1) {
        steps[0] = 1;
        return 1;
    }

    for (int i = 0;i<max_iter;i++) {
        if (input == 1) {
            return i;
        }

        steps[i] = current_val;
        input = current_val;
        current_val = collatz_conjecture(input);
    }

    return 0;
}