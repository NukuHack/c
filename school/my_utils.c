#include "my_utils.h" // self
#include <time.h> // time, random seeding

// for sleep & terminal clear
#if defined(WIN)
#  include <windows.h>
#  include <conio.h>
#else
#  include <sys/stat.h>
#  include <unistd.h>
#  include <termios.h>
#  include <sys/ioctl.h>
#endif

int my_main(int argc, char **argv) {
    while(1) {
        char **arg = shift_args(&argc, &argv);
        if (arg)
            puts(*arg);
        else
            break;
    }
    return 0;
}

#define LENGTH 20
int arr_main(void) {
    int tomb[LENGTH];

    arr_fill(ARR_L(tomb));
    arr_qsort(ARR_L(tomb));
    arr_reverse(int, tomb, ARR_S(tomb));

    arr_print(int, tomb, ARR_S(tomb));

    return 0;
}


static bool seeded = false;
void ensure_seed(void) {
    if (!seeded) {
        srand((unsigned)time(NULL));
        seeded = true;
    }
}
int randi_b(int min, int max) {
    ensure_seed();
    if (min == max)
        return min;
    if (min > max)
        mem_swap(int, &min, &max);

    ULL span = (ULL)((ULL)max - (ULL)min) + 1ULL;
    return (int)((LL)min + (LL)((ULL)rand() % span));
}
int randi(void) {
    ensure_seed();
    return (int)rand();
}
double randd(void) {
    ensure_seed();
    return (double)rand() / ((double)RAND_MAX + 1);
}
double randd_b(double min, double max) {
    ensure_seed();
    if (min == max)
        return max;

    if (min > max)
        mem_swap(double, &min, &max);

    double val = (double)rand() / ((double)RAND_MAX + 1);
    return val * (max - min) + min;
}

int cmp(int a, int b) {
    return (a > b) - (a < b);
}

void clear(void) {
    #if defined(WIN)
        system("cls");
    #else
        fputs("\033[H\033[J", stdout);
        fflush(stdout);
    #endif
}

void slep(double millis) {
#if defined(WIN)
    Sleep((DWORD)millis);
#else
    struct timespec ts;
    ts.tv_sec  = (time_t)(millis / 1000.0);
    ts.tv_nsec = (long)((millis - (double)ts.tv_sec * 1000.0) * 1e6);
    (void)nanosleep(&ts, &ts);
#endif
}

char **shift_args(int *argc, char ***argv) {
    if (!argc || !argv || *argc <= 0)
        return NULL;
    char **arg = *argv;

    // Move argv pointer forward by one element (one char*)
    argv[0]++;
    argc[0]--;
    return arg;
}

// ptr to first in alphab. ord.
char *alphabetic(char *a, char *b) {
    int r = 0;
    if (!a || !b || a == b || (r = strcmp(a, b)) == 0)
        return NULL;
    // ++ b first, -- a first, 0 = eq
    return (r > 0) ? b : a;
}

int count_words(const char *str, const char *sep) {
    if (!str || !*str || !sep || !*sep ) return 0;
    size_t initial = strspn(str, sep);
    if (initial > 0)
        str += initial; // if first char is a separator skip them
    int cnt = 0;
    while(str && *str) {
        str = strpbrk(str, sep); // find separator
          // string-span would only skip one char
          // we also skip multiple of the separators if present
          // like "apple    tree" is still 2 words
        if(str) str += strspn(str, sep); // skip separator
        ++cnt; // increment word count
    }
    return cnt;
}

void strrep(char *str, int len, char old, char upd) {
    for (--len; len >= 0; len--)
        if (str[len] == old)
            str[len] = upd;
}

// Array_Operations :

int arr_max(const int *arr, size_t len) {
    if (!arr || len == 0)
        return 0;

    int num = arr[0];
    arr_foreach(int, var, arr, len)
        if (var > num) num = var;
    return num;
}
int arr_min(const int *arr, size_t len) {
    if (!arr || len == 0)
        return 0;

    int num = arr[0];
    arr_foreach(int, var, arr, len)
        if (var < num) num = var;
    return num;
}
int arr_sum(const int *arr, size_t len) {
    int sum = 0;
    arr_foreach(int, var, arr, len)
        sum += var;
    return sum;
}


void arr_fill(int *arr, size_t len) {
    arr_foreach_p(int, var, arr, len)
        *var = randi_b(0, 100);
}

void arr_set(int *arr, size_t len, int num) {
    arr_foreach_p(int, var, arr, len)
        *var = num;
}

int cmp_int_ptr(const void *a, const void *b) {
    int x = *(const int*)a, y = *(const int*)b;
    return cmp(x, y);
}
void arr_qsort(int *arr, size_t len) {
    if (!arr || len == 0)
        return;

    qsort(arr, len, sizeof(int), cmp_int_ptr);
}



// File and other testing stuff



const char *temp_dir(void) {
#if defined(WIN)
    /* GetTempPathA requires a writable buffer; use a static one.
       NOTE: not thread-safe, and the returned pointer is only valid until the next call. */
    static char buf[MAX_PATH];
    DWORD len = GetTempPathA(MAX_PATH, buf);
    if (len == 0 || len >= MAX_PATH)
        return ".";
    return buf;
#else
    const char *dir = NULL;
    dir = getenv("TMPDIR");
    if (!dir) dir = getenv("TMP");
    if (!dir) dir = getenv("TEMP");
#if defined(P_tmpdir)
    if (!dir) dir = P_tmpdir;
#else
    if (!dir) dir = "/tmp";
#endif
    return dir;
#endif
}

/* cpu time, in sec */
double bench_cpu_now(void) {
    // some systems define clock_t as 32bit and CLOCKS_PER_SEC as 1mill
    // so we can only reliably measure "half an hour" or so, but that should be fine
    clock_t c = clock();
    if (c == (clock_t)-1)
        return -1.0;      /* clock() failure (rare) */
    /* Cast BEFORE division so 32-bit clock_t can't overflow during
       intermediate math, and "CLOCKS_PER_SEC" of "1K" or "1M" both work. */
    return (double)c / (double)CLOCKS_PER_SEC;
}

bool file_exist(const char *name) {
    FILE *file = fopen(name, "r"); // read does not create
    if (!file) {
        // could check for errno, or other thing but for now, just return
        return false;
    }
    if(fclose(file) == 0)
        return true;
    else
        // EOF returned, errno set, for now ignore
        return true;
}

