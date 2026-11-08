#include "my_utils.h" // self
#include <stddef.h> // size_t as length
#include <stdlib.h> // NULL
#include <time.h> // time, random seeding
#include <limits.h> // RAND_MAX
#include <string.h> // batch-mem op
#include <stdbool.h> // bool

// for sleep & terminal
#ifdef _WIN32
#  include <windows.h>
#  include <conio.h>
#else
#  include <unistd.h>
#  include <termios.h>
#  include <sys/ioctl.h>
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

void bubbly(int *arr, size_t len) {
    for (size_t i = 0; i < len - 1; i++)
        for (size_t j = 0; j < len - 1 - i; j++)
            if (arr+j == cmpp(arr + j, arr + j + 1))
                swap(arr + j, arr + j + 1);
}

void mem_swap(void *a, void *b, size_t size) {
    char buff[size];
    memcpy(buff, a, size);
    memcpy(a,  b, size);
    memcpy(b, buff, size);
}
void swap(int *a, int *b) {
    mem_swap(a, b, sizeof(int));
}

int *cmpp(int *a, int *b) {
    return *a >= *b ? a : b;
}

void print(const char* str) {
    if (str) fputs(str, stdout);
    fputs("\n", stdout);
}
void printi(int num) {
    fprintf(stdout, "%d\n", num);
}

void err(const char* str) {
    if (str) fputs(str, stderr);
    fputs("\n", stderr);
}

int cmp(int a, int b) {
    return (a > b) - (a < b);
}

void clear(void) {
    #ifdef _WIN32
        system("cls");
    #else
        fputs("\033[H\033[J", stdout);
        fflush(stdout);
    #endif
}

void slep(double millis) {
    #ifdef _WIN32
        Sleep(millis);
    #else
        sleep(millis / 1000);
    #endif
}


