#include "my_utils.h" // self
#include <stdlib.h> // NULL
#include <time.h> // time, random seeding
#include <limits.h> // RAND_MAX
#include <string.h> // batch-mem op

// for sleep & terminal
#if defined(_WIN32) || defined(_WIN64)
#  if !defined(WIN)
#    define   WIN
#  endif /* win-def */
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
void printn(void) {
    print(NULL);
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
    #if defined(WIN)
        system("cls");
    #else
        fputs("\033[H\033[J", stdout);
        fflush(stdout);
    #endif
}

void slep(double millis) {
    #if defined(WIN)
        Sleep(millis);
    #else
        sleep(millis / 1000);
    #endif
}

// does not work yet ... sry
char **shift_args(int *argc, char ***argv) {
    if (!argc || !argv || *argc <= 1)
        return NULL;
    char **arg = *argv;

    // Move argv pointer forward by one element (one char*)
    *argv += 1;
    *argc -= 1;
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

// taken from `https://en.cppreference.com/c/string/byte/strpbrk`
int count_words(const char *str, const char *sep) {
    int cnt = 0;
    do {
        str = strpbrk(str, sep); // find separator
          // if we use string-span instead of skip one char
          // we also skip multiple of the separators if present
          // like "apple    tree" is still 2 words
        if(str) str += strspn(str, sep); // skip separator
        ++cnt; // increment word count
    } while(str && *str);
    return cnt;
}

void strrep(char *str, int len, char old, char upd) {
    for (; len > 0; len--) {
        if (str[len] == old) {
            str[len] = upd;
        }
    }
}

#ifdef __NOT_DEFINED
// copied from `https://en.cppreference.com/c/string/byte/strtok`
// decided not to use this, since it has "complex" internal state (atleast a ptr)
// what is unreliable in concurrent or any "future" context
int main(void)
{
    char input[] = "A bird came down the walk";
    printf("Parsing the input string '%s'\n", input);
    char* token = strtok(input, " ");
    while (token)
    {
        puts(token);
        token = strtok(NULL, " ");
    }

    printf("Contents of the input string now: '");
    for (size_t n = 0; n < sizeof input; ++n)
        input[n] ? putchar(input[n]) : fputs("\\0", stdout);
    puts("'");
}
#endif

/* just for testing */
char *temp_dir(void) {
    char *tmp;
#if defined(WIN)
    char arr[1024];
    GetTempPathA(1024, arr);
#else
    char *arr;
    arr = getenv("TMPDIR");
    if(!arr) arr = getenv("TMP");
    if(!arr) arr = getenv("TEMP");
#endif
    if (!arr) arr = "/tmp";
    int l = strlen(arr); // here we assume it is a real dir and not just a '/'
    tmp = (char*)malloc(sizeof(char) * l + 1);
    if (!tmp) {
        err("malloc fail");
        return NULL;
    }
    memcpy(tmp, arr, l + 1);
    return tmp;
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

