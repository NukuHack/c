#define _POSIX_C_SOURCE 200809L
#define MY_MEMCHECK_IMPL
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
#  include <errno.h>
#  include <unistd.h>
#  include <termios.h>
#  include <sys/ioctl.h>
#endif


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
    do {
        str = strpbrk(str, sep); // find separator
          // if we use string-span instead of skip one char
          // we also skip multiple of the separators if present
          // like "apple    tree" is still 2 words
        if(str) str += strspn(str, sep); // skip separator
        ++cnt; // increment word count
    } while(str && *str);
    return cnt-1;
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
char *temp_dir(void) {
    char *tmp;
#if defined(WIN)
    char arr[1024];
    GetTempPathA(1024, arr);
#else
    const char *arr;
    arr = getenv("TMPDIR");
    if(!arr) arr = getenv("TMP");
    if(!arr) arr = getenv("TEMP");
    if (!arr) arr = "/tmp";
#endif
    size_t l = strlen(arr); // here we assume it is a real dir and not just a '/'
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

#if ZERO
size_t fwrite( const void* buffer, size_t size, size_t count, FILE* stream );
#endif

// Writes count of objects from the given array buffer to the output stream stream. The objects are written as if by reinterpreting each object as an array of unsigned char and calling fputc size times for each object to write those unsigned chars into stream, in order. The file position indicator for the stream is advanced by the number of characters written.
//
// If an error occurs, the resulting value of the file position indicator for the stream is indeterminate.
// Parameters
// buffer   -   pointer to the first object in the array to be written
// size     -   size of each object
// count    -   the number of the objects to be written
// stream   -   pointer to the output stream
// Return value
//
// The number of objects written successfully, which will be less than count only if an error occurs.
//
// If size or count is zero, fwrite returns zero and performs no other action. 

#if ZERO
size_t fread( void* buffer, size_t size, size_t count, FILE* stream );
#endif

// Reads up to count objects into the array buffer from the given input stream stream as if by calling fgetc size times for each object, and storing the results, in the order obtained, into the successive positions of buffer, which is reinterpreted as an array of unsigned char. The file position indicator for the stream is advanced by the number of characters read.
//
// If an error occurs, the resulting value of the file position indicator for the stream is indeterminate. If a partial element is read, its value is indeterminate.
// Parameters
// buffer   -   pointer to the array where the read objects are stored
// size     -   size of each object in bytes
// count    -   the number of the objects to be read
// stream   -   the stream to read
// Return value
//
// Number of objects read successfully, which may be less than count if an error or end-of-file condition occurs.
//
// If size or count is zero, fread returns zero and performs no other action.
//
// fread does not distinguish between end-of-file and error, and callers must use feof and ferror to determine which occurred. 

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

