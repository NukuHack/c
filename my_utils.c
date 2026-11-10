#include "my_utils.h" // self
#include <stdlib.h> // NULL
#include <time.h> // time, random seeding
#include <limits.h> // RAND_MAX
#include <string.h> // batch-mem op
#include <stdbool.h> // bool

#ifdef __NOT_DEFINED
// so i have sintax highlight in "if 0"
#undef __NOT_DEFINED
#endif /* __NOT_DEFINED */

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

// ptr to first in alphab. ord.
char *alphabetic(char *a, char *b) {
    int r = 0;
    if (!a || !b || a == b || (r = strcmp(a, b)) == 0)
        return NULL;
    // ++ b first, -- a first, 0 = eq
    return (r > 0) ? b : a;
#ifdef __NOT_DEFINED
    // while this works for nice (ascii a-z&A-Z) chars any other stuff
    // like numbers, non ascii chars or anything if got in it would break
    char *ap = a, *bp = b;
    for (;;ap++, bp++) {
        char ac = toupper(*ap);
        char bc = toupper(*bp);
        if (!ac || !bc) {
            if (!ac && !bc)
                return NULL; // equal
            return !ac ? a : b; // shorter comes first
        }
        if (ac != bc)
            return ac > bc ? b : a;
    }

    return NULL;
#endif
}

#ifdef __NOT_DEFINED
// taken from `https://en.cppreference.com/c/string/byte/strpbrk`
int main(void)
{
    const char* str = "hello world, friend of mine!";
    const char* sep = " ,!";

    unsigned int cnt = 0;
    do
    {
        str = strpbrk(str, sep); // find separator
        if(str) str += strspn(str, sep); // skip separator
        ++cnt; // increment word count
    }
    while(str && *str);

    printf("There are %u words\n", cnt);
}
#endif

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
#define __STDC_WANT_LIB_EXT1__ 1
#include <stdio.h>
#include <string.h>

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

  #ifdef __STDC_LIB_EXT1__ // c11 and upper, not in c99
    char str[] = "A bird came down the walk";
    rsize_t strmax = sizeof str;
    const char* delim = " ";
    char* next_token;
    printf("Parsing the input string '%s'\n", str);
    token = strtok_s(str, &strmax, delim, &next_token);
    while (token)
    {
        puts(token);
        token = strtok_s(NULL, &strmax, delim, &next_token);
    }

    printf("Contents of the input string now: '");
    for (size_t n = 0; n < sizeof str; ++n)
        str[n] ? putchar(str[n]) : fputs("\\0", stdout);
    puts("'");
  #endif
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

