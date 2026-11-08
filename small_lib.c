// self
#include "small_lib.h"
// impl
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <stdbool.h>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <unistd.h>
    #include <termios.h>
    #include <sys/ioctl.h>
#endif

static bool seeded = false;
int rand_int(int min, int max) {
    if (!seeded) {
        srand((unsigned)time(NULL));
        seeded = true;
    }

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
    if (!seeded) {
        srand((unsigned)time(NULL));
        seeded = true;
    }

    return (double)rand() / ((double)RAND_MAX + 1);
}

void print(const char* str) {
    if (str) fputs(str, stdout);
    fputs("\n", stdout);
}

void clear(void) {
    #ifdef _WIN32
        system("cls");
    #else
        fputs("\033[H\033[J", stdout);
        fflush(stdout);
    #endif
}

void slep(int millis) {
    #ifdef _WIN32
        Sleep(millis);
    #else
        sleep(millis / 1000);
    #endif
}

void err(const char* str) {
    if (str) fputs(str, stderr);
    fputs("\n", stderr);
}

int cmp(int a, int b) {
    return (a > b) - (a < b);
}

