#define _POSIX_C_SOURCE 200809L
#include "my_utils.h" // self
#include <time.h> // time, random seeding
#include <limits.h> // RAND_MAX
#include <errno.h> // perror, and errno, Cool

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
            print(*arg);
        else
            break;
    }
    return 0;
}

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

void err(const char* str) {
    if (str) fputs(str, stderr);
    fputs("\n", stderr);
}

int cmp(int a, int b) {
    return (a > b) - (a < b);
}
int cmp_int_ptr(const void *a, const void *b) {
    int x = *(const int*)a, y = *(const int*)b;
    return cmp(x, y);
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
    *argv += 1;
    *argc -= 1;
    return arg;
}

bool resize_buffer(void **bufptr, size_t *n) {
    if (!bufptr) {
        errno = EINVAL;
        return false;
    }
    if (*n > (size_t)-1 / 2) {
        errno = ENOMEM;
        return false;
    }
    size_t new_size = *n ? *n * 2 : BUF_LEN;
    void *p = (void*)realloc(*bufptr, new_size);
    if (!p) {
        errno = ENOMEM;
        return false;
    }
    *bufptr = p;
    *n = new_size;
    return true;
}

bool resize_buff_t(void **bufptr, size_t *n, size_t size) {
    if (!bufptr || size == 0) {
        errno = EINVAL;
        return false;
    }
    if (*n > (size_t)-1 / 2 || size > (size_t)-1/2) {
        errno = ENOMEM;
        return false;
    }
    size_t new_size = (*n ? *n * 2 : BUF_LEN);
    void *p = (void*)realloc(*bufptr, new_size * size);
    if (!p) {
        errno = ENOMEM;
        return false;
    }
    *bufptr = p;
    *n = new_size;
    return true;
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
    int cnt = 0;
    if (!str || !*str) return 0;
    size_t initial = strspn(str, sep);
    if (initial > 0)
        str += initial; // if first char is a separator skip them
    do {
        str = strpbrk(str, sep); // find separator
          // string-span would only skip one char
          // we also skip multiple of the separators if present
          // like "apple    tree" is still 2 words
        if(str) str += strspn(str, sep); // skip separator
        ++cnt; // increment word count
    } while(str && *str);
    return cnt;
}

void strrep(char *str, int len, char old, char upd) {
    for (--len; len >= 0; len--)
        if (str[len] == old)
            str[len] = upd;
}

/* just for testing */
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

/**
 * FILE stuff :
 * */

bool check_file_exist(const char *name) {
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

