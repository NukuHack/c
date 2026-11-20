#include "my_io.h" // self
#include "my_utils.h" // basic io
#include <limits.h> // limit for overflow checking
#include <math.h> // double stuff for overflow & NAN
#include <ctype.h> // basic char methods

#define EOF_RETRY "closed input, but retry\n"

int input_main(void) {
    int size = inpi_s("inp number [0,10]: ", 0, 10);
    printf("got: %d\n", size);

    char* str = inps();
    print(str);
    destroy(str);

    char c = inpc();
    printf("got char : %c\n", c);

    double d = inpd();
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
void handle_eof(FILE *in, FILE *out) {
    clearerr(in);
    if (!out) return;
    fputc('\n', out);
    fflush(out);
}
void drain_line(FILE *in, FILE *out) {
    int c;
    while ((c = fgetc(in)) != '\n' && c != '\r' && c != EOF);

    if (c == '\r' && (c = fgetc(in)) != '\n' && c != EOF)
        ungetc(c, in);

    /* we ran out of input before any line terminator -> the user's
       cursor is still sitting at the end of the current line, so
       emit one ourselves to keep prompts/errors on their own line */
    if (c == EOF) {
        handle_eof(in, out);
    }
}
void trim_n(char *buf, size_t *bufs) {
    size_t l = *bufs;
    while(l > 0 && ((buf)[l - 1] == '\n' || (buf)[l - 1] == '\r'))
        buf[--l] = '\0';
    *bufs = l;
}




// basic wrapper for fgets
ptrdiff_t fgetstack(char **bufptr, size_t *n, FILE *in) {
    // no need for complex logic, since who would allocate memory on stack for an Entire file ...
    if (!bufptr || !n || !in || !*bufptr || *n < 2) { /* need room for atleast 1 char + NUL */
        errno = EINVAL;
        return -1;
    }
    /* fgets takes int; clamp so huge buffers don't overflow. */
    int chunk = CLAMP(*n, (size_t)INT_MAX, int);

    if (!fgets(*bufptr, chunk, in))
        return ferror(in) ? -1 : 0; /* distinguish error from EOF */

    char *nul = (char*)memchr(*bufptr, '\0', *n);
    if (!nul) {
        errno = EOVERFLOW;
        return -1;
    }
    size_t len = (size_t)(nul - *bufptr);

    if (len == 0 && ferror(in))
        return -1;

    /* (*bufptr)[len] is already '\0' - by fgets */
    if (len > (size_t)PTRDIFF_MAX) {
        errno = EOVERFLOW;
        return -1;
    }
    return (ptrdiff_t)len;
}
ptrdiff_t fgetbatch(char **bufptr, size_t *n, FILE *in, bool stpN) {
    if (!bufptr || !n || !in) {
        errno = EINVAL;
        return -1; // wrong args
    }
    // we can use the same "realloc - resize", realloc "NULL" is just a malloc
    char *p = *bufptr;
    if ((!p || *n < 4u) && !(p = (char*)resize_buff(p, n, sizeof(char))))
        return -1; // malloc failed 
    else
        *bufptr = p;

    size_t len = 0;
#if !ZERO
    // simplest version : dynamic mem alloc - handles null input 
    // char-by-char so any extension / refinement is easy to do 
    // os-independent, nothing like "getline" what only works "here and there" 
    while (1) {
        /* need room for next char + the NUL terminator */
        IF_GROW(*bufptr, n, len, char)
            return -1; // realloc failed

        int c = fgetc(in);
        if (c == EOF)
          break; /* EOF or error */

        (*bufptr)[len++] = (char)c;
        if (c == '\n' && stpN)
            break; /* end of line */
    }
    goto ending;
#else // this is dynamic alloc too, moved static stuff to "stack" method, since no mem resize
    if (!stpN) {
        while (1) {
            IF_GROW(*bufptr, n, len, char)
                return -1; // realloc failed

            /* -1 reserves the NUL slot. fread never writes past it. */
            size_t got = fread(*bufptr + len, 1, *n - len - 1, in);
            if (got == 0)
                break; /* EOF or error */
            len += got;
        }
        goto ending;
    }
    // fread is bad, since it does not stop at \n
    // so it's easier to just use fgets() since it stops at EOF and newline nicely
    while (1) {
        /* Room for at least 1 char + NUL. */
        IF_GROW(*bufptr, n, len, char)
            return -1; // realloc failed

        size_t avail = *n - len;    /* room including NUL */
        int chunk = (avail > (size_t)INT_MAX) ? INT_MAX : (int)avail;

        if (!fgets(*bufptr + len, chunk, in))
            break; /* EOF or error */

        /* fgets guarantees a NUL; memchr is bounded, unlike strlen, so no segfault. */
        char *nul = (char*)memchr(*bufptr + len, '\0', (size_t)chunk);
        size_t written = (size_t)(nul - (*bufptr + len));
        len += written;

        if (written > 0 && (*bufptr)[len - 1] == '\n')
            break; /* got the line */
    }
    goto ending;
#endif
ending:;;

    /* important by all means, should not remove !! */
    (*bufptr)[len] = '\0';

    /* Not eof, but error */
    // maybe simply just ferror(in) would be fine, but then we would lose the partial input
    if (len == 0 && ferror(in))
        return -1;

    if (len > (size_t)PTRDIFF_MAX) {
        /*  *sher sound* "like that's ever gonna happen" */
        errno = EOVERFLOW;
        return -1;
    }
    return (ptrdiff_t)len;
}

ptrdiff_t fputbatch(char **bufptr, size_t *n, FILE *out, bool stpN) {
    if (!bufptr || !*bufptr || !n || *n == 0 || !out) {
        return -1;
    }
    size_t write_len = *n * sizeof(char);
    if (stpN) {
        char *nl = (char*)memchr(*bufptr, '\n', *n); // Search for '\n'
        if (nl != NULL) {
            // Calculate length including '\n'
            write_len = (size_t)((char *)nl - *bufptr) + 1;
        }
    }

    size_t pr = fwrite(*bufptr, sizeof(char), write_len, out);
    // "If an error occurs, the resulting value of the file position indicator for the stream is indeterminate."
    // Return value :
    // The number of objects written successfully, which will be less than count only if an error occurs.
    // - so no need for loops an retry, since it's IO error
    return CLAMP(pr, (size_t)PTRDIFF_MAX, ptrdiff_t);
}


int inpi(void) {
    return inpi_s("input an int: ", -10000, 10000);
}
uint inpu(void) {
    return inpu_s("input an unsigned: ", 0, 10000);
}
double inpd(void) {
    return inpd_s("input a double: ", -10000, 10000);
}
char inpc(void) {
    return inpc_s("input an alphabetic char: ", (predicate*)isalpha);
}
char* inps(void) {
    return inps_s("input a line: \n");
}

/* re-try inputs and easy use */

int inpi_s(const char* prompt, int min, int max) {
    if (min > max) { int t = min; min = max; max = t; }

    int val;
    char buf[BUF_LEN];
read:;
    if (!read_str(prompt, ARR_L(buf)))
        goto read;
    if (strlen(buf) == (size_t)parse_i(buf, &val)) {
        if (val >= min && val <= max)
            return val;
        else
            printf("input a number in [%d,%d]\n", min, max);
    } else {
        printf(EOF_RETRY);
    }
    goto read;
}

uint inpu_s(const char* prompt, uint min, uint max) {
    if (min > max) { uint t = min; min = max; max = t; }

    uint val;
    char buf[BUF_LEN];
read:;
    if (!read_str(prompt, ARR_L(buf)))
        goto read;
    if (strlen(buf) == (size_t)parse_u(buf, &val)) {
        if (val >= min && val <= max)
            return val;
        else
            printf("input a number in [%u,%u]\n", min, max);
    } else {
        printf(EOF_RETRY);
    }
    goto read;
}

double inpd_s(const char* prompt, double min, double max) {
    if (min > max) { double t = min; min = max; max = t; }

    double val;
    char buf[BUF_LEN];
read:;
    if (!read_str(prompt, ARR_L(buf)))
        goto read;
    if (strlen(buf) == (size_t)parse_d(buf, &val)) {
        if (isfinite(val) && val >= min && val <= max)
            return val;
        else
            printf("input a number in [%f,%f]\n", min, max);
    } else {
        printf(EOF_RETRY);
    }
    goto read;
}

char inpc_s(const char* prompt, predicate *filter) {

    char val;
    char buf[BUF_LEN];
read:;
    if (!read_str(prompt, ARR_L(buf)))
        goto read;
    if ((val = *buf) != '\0') {
        if (buf[1] == '\0' && filter && val >= 0 && filter(val))
            return val;
        else
            printf("wrong char [not allowed]\n");
    } else {
        printf(EOF_RETRY);
    }
    goto read;
}

char *inps_s(const char *prompt) {
    size_t bufs = BUF_LEN;
    char *buf = New(char, bufs);
    if (bufs <= 1 || !buf)
        return NULL;
    if (prompt) {
        fputs(prompt, stdout);
        fflush(stdout);
    }

    ptrdiff_t l = fgetbatch(&buf, &bufs, stdin, true);
    if (feof(stdin)) {
        handle_eof(stdin, stdout);
    } else if (ferror(stdin)) {
        handle_eof(stdin, stdout);
        fprintf(stdout, "BIG ERROR err:%d\n", errno);
        goto ending;
    } else if (l == -1) {
        fprintf(stdout, "failed err:%d\n", errno);
        goto ending;
    }
    trim_n(buf, (size_t*)&l);
    return buf;

ending:;;
    free(buf);
    return NULL;
}
char *read_str(const char *prompt, char *buf, size_t bufs) {
    if (bufs <= 1 || !buf)
        return NULL;
    if (prompt) {
        fputs(prompt, stdout);
        fflush(stdout);
    }

    ptrdiff_t l = fgetstack(&buf, &bufs, stdin);
    if (!feof(stdin) && l > 0 && (buf)[l - 1] != '\n') {
        drain_line(stdin, NULL); // consume the rest of the line
    }
    if (feof(stdin)) {
        handle_eof(stdin, stdout);
    } else if (ferror(stdin)) {
        handle_eof(stdin, stdout);
        fprintf(stdout, "BIG ERROR err:%d\n", errno);
        goto ending;
    } else if (l == -1) {
        fprintf(stdout, "failed err:%d\n", errno);
        goto ending;
    }
    trim_n(buf, (size_t*)&l);
    return buf;

ending:;;
    return NULL;
}





/* parsers */
// since they already have the string in memory going char-by-char is worse, so just use the standard
// "strto_" methods to convert from string to numbers nicely
int parse_i(const char *text, int *num) {
    if (!text || !num) return -1;
    char *endptr;
    errno = 0;
    long v = strtol(text, &endptr, 10);
    if (endptr == text) return 0;
    if (errno == ERANGE || v < INT_MIN || v > INT_MAX) return -1;
    *num = (int)v;
    return (int)(endptr - text);
}

int parse_d(const char *text, double *num) {
    if (!text || !num) return -1;
    char *endptr;
    errno = 0;
    double v = strtod(text, &endptr);
    if (endptr == text) return 0;
    if (errno == ERANGE) return -1;
    *num = v;
    return (int)(endptr - text);
}

int parse_u(const char *text, uint *num) {
    if (!text || !num) return -1;
    // no negative
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


