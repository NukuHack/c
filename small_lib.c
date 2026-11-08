// self
#include "small_lib.h"
// impl
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#define STR_BUF 100

int lib_main(void) {
    int size = inp_int_s("number: ", 0, 10);
    printf("got: %d\n", size);

    char* str = inp_str();
    print(str);
    free(str);
    str = NULL;

    char c = inp_char();
    printf("got char : %c\n", c);

    double d = inp_double();
    printf("double: %lf\n", d);

    print("10 random double:");
    for (int i = 0; i < 10; i++) {
        printf("%d), %lf\n", i, rand_double());
    }

    return 0;
}

int rand_int(int min, int max) {
    if (min == max)
        return max;

    if (min > max) {
        int t = min;
        min = max;
        max = t;
    }

    long r = (long)rand() % (long)(max - min + 1);
    return (int)r + min;
}

double rand_double(void) {
    return (double)rand() / ((double)RAND_MAX + 1);
}

void print(const char* str) {
    if (str) fputs(str, stdout);
    fputs("\n", stdout);
}

void clear(void) {
    fputs("\033[H\033[J", stdout);
    fflush(stdout);
}

void err(const char* str) {
    if (str) fputs(str, stderr);
    fputs("\n", stderr);
}

int cmp(int a, int b) {
    return (a > b) - (a < b);
}

