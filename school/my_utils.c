#define _POSIX_C_SOURCE 200809L
#include "my_utils.h" // self
#include <time.h> // time, random seeding
#include <limits.h> // RAND_MAX

// for sleep & terminal
#if defined(WIN)
#  include <windows.h>
#  include <conio.h>
#else
#  include <errno.h>
#  include <unistd.h>
#  include <termios.h>
#  include <sys/ioctl.h>
#endif

const char *const units[] = {"B","KB","MB","GB","TB","PB","EB"};

int my_main(int argc, char **argv) {
    do {
        char **arg = shift_args(&argc, &argv);
        print(*arg);
    } while (argc > 0);
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
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
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

char *fmt_num(LLU v, char *buf, size_t buflen) {
    int MAX_TIER = ARR_S(units)-1; /* index of the max */

    if (!buf || buflen == 0)
        return buf;
    if (v < 1000ULL) {
        snprintf(buf, buflen, "%llu B", v);
        return buf;
    }

    /* 1. Find tier: d = 1000^tier, the largest power with v / d >= 1.
     *    1000^6 = 1e18 fits in 64 bits, and v/1e18 <= 18, so the loop
     *    can never run past EB. */
    unsigned long long d = 1ULL;
    int tier = 0;
    while (tier < MAX_TIER && v / d >= 1000ULL) { d *= 1000ULL; ++tier; }

    unsigned long long ip  = v / d;             /* integer part, 1..999 */
    unsigned long long rem = v % d;             /* remainder, < d <= 1e18 */

    /* 2. 5 significant digits total. */
    int decimals = 5 - ((ip >= 100) ? 3 : (ip >= 10) ? 2 : 1);

    /* 3. Exact long division for the fractional digits.
     *    rem * 10 < 1e19 < 2^64, so it never overflows. */
    unsigned frac = 0, pow10 = 1;
    for (int i = 0; i < decimals; ++i) {
        rem *= 10;
        frac = frac * 10 + (unsigned)(rem / d);
        rem %= d;
        pow10 *= 10;
    }

    /* 4. Round half up on the exact remainder, then propagate carries. */
    if (rem * 2 >= d && ++frac == pow10) {
        frac = 0;
        if (++ip == 1000ULL && tier < MAX_TIER) { ip = 1; ++tier; }
    }

    /* 5. Fraction digits, trailing zeros trimmed. */
    char dig[4];
    for (int i = decimals - 1; i >= 0; --i) { dig[i] = (char)('0' + frac % 10); frac /= 10; }
    while (decimals > 0 && dig[decimals - 1] == '0') --decimals;

    if (decimals)
        snprintf(buf, buflen, "%llu.%.*s %s", ip, decimals, dig, units[tier]);
    else
        snprintf(buf, buflen, "%llu %s", ip, units[tier]);
    return buf;
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
    if (!str || !*str) return 0;
    if (*str == ' ') cnt--;
    do {
        str = strpbrk(str, sep); // find separator
          // string-span would only skip one char
          // we also skip multiple of the separators if present
          // like "apple    tree" is still 2 words
        if(str) str += strspn(str, sep); // skip separator
        ++cnt; // increment word count
    } while(str && *str);
    return cnt == -1 ? 0 : cnt;
}

void strrep(char *str, int len, char old, char upd) {
    for (--len; len > 0; len--) {
        if (str[len] == old) {
            str[len] = upd;
        }
    }
}

#if ZERO
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
       intermediate math, and "CLOCKS_PER_SEC" of 1e3 or 1e6 both work. */
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

