#include "input.h" // self
#include "my_utils.h" // basic io
#include <limits.h> // limit for overflow checking
#include <math.h> // double stuff for overflow & NAN
#include <errno.h> // error checking mainly "EOF" and "errno" at builtin stuff
#include <ctype.h> // basic char methods

#define IO stdin,stdout
#define destroy(a) do { free(a);a=NULL; }while(0)
#define EOF_RETRY "closed input, but retry\n"

int input_main(void) {
    int size = inp_int_s("inp number [0,10]: ", 0, 10);
    printf("got: %d\n", size);

    char* str = inp_str();
    print(str);
    destroy(str);

    char c = inp_char();
    printf("got char : %c\n", c);

    double d = inp_double();
    printf("double: %.3lf\n", d);

    print("10 random double:");
    for (int i = 0; i < 10; i++) {
        printf("%d), %lf\n", i, rand_double());
    }

    return 0;
}

/* helpers */

void consume_crlf_tail(FILE *in) {
    int c = fgetc(in);
    if (c == EOF) return;
    if (c != '\n') ungetc(c, in);
}


void new_line(FILE *out) {
    if (!out) return;
    fputc('\n', out);
    fflush(out);
}

void handle_eof(FILE *in, FILE *out) {
    clearerr(in);
    new_line(out);
}


// make it not prtint error, and move re-try outside
// TODO : make it return "inp_res"
void input_char(FILE *in, FILE *out, char *v, predicate *filter, const char *prompt) {
#define retry_text "input a char (char)\n"
#define not_allow "wrong char [not allowed]\n"
    int c, ch;
read:
    if (prompt && out) { fputs(prompt, out); fflush(out); }

    ch = fgetc(in);
    if (ch == EOF) {
        handle_eof(in, out);
        fprintf(out, EOF_RETRY);
        goto read;
    }

    if (ch == '\n' || ch == '\r') {
        if (ch == '\r') consume_crlf_tail(in);
        fprintf(out, retry_text);
        goto read;
    }

    c = fgetc(in);
    if (c == '\r') {
        consume_crlf_tail(in);
        c = '\n';
    }

    if (c != '\n' && c != EOF) { // EOF case
        drain_line(in, out);
        fprintf(out, retry_text);
        goto read;
    }

    if (filter && !filter(ch)) {
        fprintf(out, not_allow);
        goto read;
    }

    *v = (char)ch;
}


void input_string(FILE *in, FILE *out, char **buf, size_t len, const char *prompt) {
    if (prompt && out) {
        fputs(prompt, out);
        fflush(out);
    }
    if (!in || !len || !buf)
        return;

    ptrdiff_t l;
    l = fgetline(buf, &len, in);
    if (l == EOF && feof(in)) {
        handle_eof(in, out);
    } else if (l == -1 && ferror(in)) {
        fprintf(out, "BIG ERROR\n");
    } else if (l == -1) {
        fprintf(out, "failed\n");
    }
    return;
}



/*
 * Reads one line from `FILE *in` (any length), including the trailing "new-line" if present.
 * The buffer pointed to by `*bufptr` is allocated/reallocated as needed and is
 * always "\0"-terminated on success.
 *
 * On success : returns number of bytes read (excluding the "\0").
 * On EOF     : returns -1 with no data consumed; errno unchanged.
 * On error   : returns -1 and sets errno (EINVAL / ENOMEM / whatever fgetc set).
 *
 * The caller owns `*bufptr` and must free() it (possible "NULL").
 * *bufptr must be "NULL", or a pointer from malloc/realloc (with *n its size).
 * If it is stack allocated resize is "Undefined" best case is crash.
 */
ptrdiff_t fgetline(char **bufptr, size_t *n, FILE *in) {
    ptrdiff_t r = fgetbatch(bufptr, n, in, true);
    return r;
}
ptrdiff_t fgetfile(char **bufptr, size_t *n, FILE *in) {
    ptrdiff_t r = fgetbatch(bufptr, n, in, false);
    return r;
}
ptrdiff_t fgetbatch(char **bufptr, size_t *n, FILE *in, bool stpN) {
    if (!bufptr || !n || !in) {
        errno = EINVAL;
        return -1;
    }
    if (!*bufptr) *n = 0;

    size_t len = 0;
#if defined(DYN_STR)
    int c;
    while ((c = fgetc(in)) != EOF) {
        /* need room for this char + the NUL terminator */
        if (len + 1 >= *n) {
            if (*n == 0) { /* if size is 0 */
                *n = BUF_LEN >> 1; /* half size */
            } else if (*n > (size_t)-1 / 2) { /* overflow guard */
                ungetc(c, in); /* restore consumed char */
                errno = ENOMEM;
                return -1;
            }
            size_t new_size = *n * 2;
            char *p = (char*)realloc(*bufptr, new_size);
            if (p == NULL) {
                ungetc(c, in); /* restore consumed char */
                errno = ENOMEM;
                return -1; /* ENOMEM, *lineptr intact */
            }
            *bufptr = p;
            *n = new_size;
        }
        (*bufptr)[len++] = (char)c;
        if (c == '\n' && stpN) break; /* end of line */
    }
#elif ZERO // this would be nice, but needs even more work to handle \n in the middle
    if (!b || *n <= 1) return 0;
    size_t read;
    while ((read = fread(*bufptr + len, sizeof(char), (*n - len) - 1, in)) != 0) {
        len += read;
    }
#else /* basic wrapper of fgets */
    char *b = *bufptr; // still ptr, so points to same val
    if (!b || *n <= 1) return 0;
    if (!fgets(b, (int)*n, in)) return -1;
    len = strlen(b);
#endif

    if (ferror(in)) {
        /* File error */
        return -1;
    }
    if (len == 0) {
        /* Nothing read: EOF (feof) */
        return (ptrdiff_t)EOF;
    }

    (*bufptr)[len] = '\0';
    return (ptrdiff_t)len;
}


// high level inputs have a min-max of 10K, reason being ... nothing
// usually we take in that kind of numbers, if a number is over 10k the caller should be explicit

int inp_int(void) {
    return inp_int_s("input an int: ", -10000, 10000);
}
uint inp_uint(void) {
    return inp_uint_s("input an unsigned: ", 0, 10000);
}
double inp_double(void) {
    return inp_double_s("input a double: ", -10000, 10000);
}
char inp_char(void) {
    return inp_char_s("input an alphabetic char: ", (predicate*)isalpha);
}
char* inp_str(void) {
    return inp_str_s("input a line: \n");
}

/* re-try inputs and easy use */

int inp_int_s(const char* text, int min, int max) {
    if (min > max) { int t = min; min = max; max = t; }

    int val, ret;
read:;
    if (text) {
        fputs(text, stdout);
        fflush(stdout);
    }
    ret = scanf("%d ", &val);
    if (ret == 1) {
        if (val >= min && val <= max)
            return val;
        else
            printf("input a number in [%d,%d]\n", min, max);
    } else {
        printf(EOF_RETRY);
    }
    goto read;
}

uint inp_uint_s(const char* text, uint min, uint max) {
    if (min > max) { uint t = min; min = max; max = t; }

    uint val; int ret;
read:;
    if (text) {
        fputs(text, stdout);
        fflush(stdout);
    }
    ret = scanf("%u ", &val);
    if (ret == 1) {
        if (val >= min && val <= max)
            return val;
        else
            printf("input a number in [%u,%u]\n", min, max);
    } else {
        printf(EOF_RETRY);
    }
    goto read;
}

double inp_double_s(const char* text, double min, double max) {
    if (min > max) { double t = min; min = max; max = t; }

    double val; int ret;
read:;
    if (text) {
        fputs(text, stdout);
        fflush(stdout);
    }
    ret = scanf("%lf ", &val);
    if (ret == 1) {
        if (isfinite(val) && val >= min && val <= max)
            return val;
        else
            printf("input a number in [%f,%f]\n", min, max);
    } else {
        printf(EOF_RETRY);
    }
    goto read;
}

char inp_char_s(const char* text, predicate *filter) {
    char c;
    input_char(stdin, stdout, &c, filter, text);
    return c;
}
char *inp_str_s(const char *text) {
    char *buf = (char*)malloc(BUF_LEN);
    if (!buf) // fail
        return NULL;
    input_string(stdin, stdout, &buf, BUF_LEN, text);
    return buf;
}
char *read_str(const char *msg, char *buf, size_t bufs) {
    input_string(stdin, stdout, &buf, bufs, msg);
    return buf;
}




#if ZERO
size_t fwrite(const void* buffer, size_t size, size_t count, FILE* stream);
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




/* parsers */
// since they already have the string in memory going char-by-char is worse, so just use the standard
// "strto_" methods to convert from string to numbers nicely
int parse_int(const char *text, int *num) {
    if (!text || !num) return -1;
    char *endptr;
    errno = 0;
    long v = strtol(text, &endptr, 10);
    if (endptr == text) return 0;
    if (errno == ERANGE || v < INT_MIN || v > INT_MAX) return -1;
    *num = (int)v;
    return (int)(endptr - text);
}

int parse_double(const char *text, double *num) {
    if (!text || !num) return -1;
    char *endptr;
    errno = 0;
    double v = strtod(text, &endptr);
    if (endptr == text) return 0;
    if (errno == ERANGE) return -1;
    *num = v;
    return (int)(endptr - text);
}

int parse_uint(const char *text, uint *num) {
    if (!text || !num) return -1;

    // Reject negative numbers explicitly
    const char *p = text;
    while (isspace((unsigned char)*p)) p++;
    if (*p == '-') return -1;

    char *endptr;
    errno = 0;
    unsigned long v = strtoul(text, &endptr, 10);
    if (endptr == text) return 0;
    if (errno == ERANGE || v > UINT_MAX) return -1;
    *num = (uint)v;
    return (int)(endptr - text);
}


