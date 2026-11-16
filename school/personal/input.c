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

bool finish_number(FILE *in, int c) {
    if (c == EOF || c == '\n')
        return false;
    if (c == '\r') {
        consume_crlf_tail(in);
        return false;
    }
    ungetc(c, in);
    return true;
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
bool input_rejected(inp_res s) {
    // reject everything that is not Good or Error
    return !(s == INPUT_OK || s == INPUT_EOF || INPUT_IOERR);
}
bool input_drain_required(inp_res s) {
    switch (s) {
        default:
        case INPUT_EOF:
        case INPUT_IOERR:
        case INPUT_EMPTY:
        case INPUT_OK: return false;
        case INPUT_SEP:
        case INPUT_NAN:
        case INPUT_OVERFLOW:
        case INPUT_RANGE:
        case INPUT_STOPPED: return true;
    }
}
const char *input_status_str(inp_res s) {
    switch (s) {
        case INPUT_OK: return "ok"; // 123
        case INPUT_SEP: return "separator not found"; // 123;123 instead of 123-123
        case INPUT_NAN: return "not a number"; // apple
        case INPUT_EOF: return "end of input"; // EOF
        case INPUT_IOERR: return "io error"; // ferror
        case INPUT_EMPTY: return "empty"; //  \n  -  \r\n
        case INPUT_RANGE: return "out of range"; // [10;40] - 300
        case INPUT_STOPPED: return "stopped (partially usable input)"; // 1234a
        case INPUT_OVERFLOW: return "overflow"; // more than 2^64 ...
        default: return "unknown";
    }
}

/* char-by-char read + parse -> no malloc */
// usually parsers work like this :
// go through the buffer and try to parse it
// but in fact we can use the FILE stream as a buffer here, and go by char-s to have the "same" result
// but since we can't look ahead without destroying too much we can't parse special double cases
// - stuff like "nan" and "inf" or "infinity" is just errors now ...

inp_res input_int(FILE *in, FILE *out, int *v, int min, int max, const char *prompt) {
    if (min > max) { int t = min; min = max; max = t; }

    if (prompt && out) { fputs(prompt, out); fflush(out); }

    int c = fgetc(in);
    if (c == EOF)
        return INPUT_EOF;
    if (c == '\n' || c == '\r') {
        if (c == '\r') consume_crlf_tail(in);
        return INPUT_EMPTY;
    }

    int sign = 1;
    if (c == '+' || c == '-') {
        if (c == '-') sign = -1;
        c = fgetc(in);
        if (c == EOF)
            return INPUT_EOF;
        if (c == '\n' || c == '\r') {
            ungetc(c, in);
            return INPUT_NAN;
        }
        if (c < '0' || c > '9') {
            ungetc(c, in);
            return INPUT_NAN;
        }
    } else if (c < '0' || c > '9') {
        ungetc(c, in);
        return INPUT_NAN;
    }

    long long n = 0;
    bool overflow = false;

    do {
        int d = c - '0';
        if (!overflow) {
            long long limit = (sign > 0)
                ? INT_MAX
                : (long long)INT_MAX + 1;

            if (n > (limit - d) / 10)
                overflow = true;
            else
                n = n * 10 + d;
        }
    } while ((c = fgetc(in)) >= '0' && c <= '9');

    long long signed_n = n * sign;

    if (overflow) {
        if (c != EOF) ungetc(c, in);
        return INPUT_OVERFLOW;
    } else if (signed_n < min || signed_n > max) {
        if (c != EOF) ungetc(c, in);
        return INPUT_RANGE;
    }

    if (c == EOF) {
        clearerr(in);
        new_line(out);
    }
    bool stopped = finish_number(in, c);
    if (v) *v = (int)signed_n;
    return stopped ? INPUT_STOPPED : INPUT_OK;
}

inp_res input_uint(FILE *in, FILE *out, uint *v, uint min, uint max, const char *prompt) {
    if (min > max) { uint t = min; min = max; max = t; }

    if (prompt && out) { fputs(prompt, out); fflush(out); }

    int c = fgetc(in);
    if (c == EOF) return INPUT_EOF;
    if (c == '\n' || c == '\r') {
        if (c == '\r') consume_crlf_tail(in);
        return INPUT_EMPTY;
    }

    if (c == '+') {
        c = fgetc(in);
        if (c == EOF)
            return INPUT_EOF;
        if (c == '\n' || c == '\r') {
            ungetc(c, in);
            return INPUT_NAN;
        }
        if (c < '0' || c > '9') {
            ungetc(c, in);
            return INPUT_NAN;
        }
    } else if (c < '0' || c > '9') {
        ungetc(c, in);
        return INPUT_NAN;
    }

    uint n = 0;
    bool overflow = false;

    do {
        uint d = (uint)(c - '0');
        if (!overflow) {
            if (n > (UINT_MAX - d) / 10u)
                overflow = true;
            else
                n = n * 10u + d;
        }
    } while ((c = fgetc(in)) >= '0' && c <= '9');

    if (overflow) {
        if (c != EOF) ungetc(c, in);
        return INPUT_OVERFLOW;
    } else if (n < min || n > max) {
        if (c != EOF) ungetc(c, in);
        return INPUT_RANGE;
    }

    if (c == EOF) {
        clearerr(in);
        new_line(out);
    }
    bool stopped = finish_number(in, c);
    if (v) *v = n;
    return stopped ? INPUT_STOPPED : INPUT_OK;
}

inp_res input_double(FILE *in, FILE *out, double *v, double min, double max, const char *prompt) {
    if (min > max) { double t = min; min = max; max = t; }

    if (prompt && out) { fputs(prompt, out); fflush(out); }

    int c = fgetc(in);
    if (c == EOF) return INPUT_EOF;
    if (c == '\n' || c == '\r') {
        if (c == '\r') consume_crlf_tail(in);
        return INPUT_EMPTY;
    }

    int sign = 1;
    if (c == '+' || c == '-') {
        if (c == '-') sign = -1;
        c = fgetc(in);
        if (c == EOF) return INPUT_EOF;
        if (c == '\n' || c == '\r') {
            ungetc(c, in);
            return INPUT_NAN;
        }
        if ((c < '0' || c > '9') && c != '.') {
            ungetc(c, in);
            return INPUT_NAN;
        }
    } else if ((c < '0' || c > '9') && c != '.') {
        ungetc(c, in);
        return INPUT_NAN;
    }

    double int_part = 0.0;
    double frac_part = 0.0;
    double frac_scale = 1.0;
    int norm_digit = 0;
    int frac_digits = 0;

    while (c >= '0' && c <= '9') {
        // if (norm_digit < 20) {
        // while double only keeps perfect precission up to digit ~16 i decided to let go of that
        // so this will parse insainly big (~1e300, aka just barely under inf) big doubles too
        int_part = int_part * 10.0 + (c - '0');
        norm_digit++;
        // }
        c = fgetc(in);
    }
    if (c == '.') {
        c = fgetc(in);
        while (c >= '0' && c <= '9') {
            if (frac_digits < 20) {
                /* double keeps precission up to 16 digits, 20 should be fine for decimal places */
                frac_part = frac_part * 10.0 + (c - '0');
                frac_scale *= 10.0;
                frac_digits++;
            }
            c = fgetc(in);
        }
    }

    if (!norm_digit && !frac_digits) {
        if (c != EOF) ungetc(c, in);
        return INPUT_NAN;
    }

    double d = (int_part + frac_part / frac_scale) * sign;

    if (isinf(d)) {
        // in a real double input infinity would be acceptable, but here it is not
        // since we don't parse strings infinity can only occur at overflow
        if (c != EOF) ungetc(c, in);
        return INPUT_OVERFLOW;
    } else if (d < min || d > max) {
        if (c != EOF) ungetc(c, in);
        return INPUT_RANGE;
    }

    if (c == EOF) {
        clearerr(in);
        new_line(out);
    }
    bool stopped = finish_number(in, c);
    if (v) *v = d;
    return stopped ? INPUT_STOPPED : INPUT_OK;
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


#if defined(DYN_STR) /* Dynamic-resize string */

typedef struct String_t {
    char *data;
    size_t len;
    size_t sz;
} String;

String* sv_create(void) {
    String* v = (String*)malloc(sizeof *v);
    if (!v) return NULL;
    v->data = NULL;
    v->len = 0;
    v->sz = 0;
    return v;
}
String* sv_create_frm(char *buf, size_t len) {
    String* v = (String*)malloc(sizeof *v);
    if (!v) return NULL;
    v->data = buf;
    v->len = 0;
    v->sz = len;
    return v;
}

void sv_free(String* v) {
    if (!v) return;
    if (v->data)
        free(v->data);
    free(v);
}
char* sv_free_get(String* v) {
    if (!v) return NULL;
    char* d = v->data;
    free(v);
    return d;
}

bool sv_reserve(String *v, size_t need) {
    if (!v) return false;
    if (need <= v->sz) return true;

    size_t ncap = v->sz ? v->sz * 2 : BUF_LEN;
    while (ncap < need) ncap <<= 1;

    char *p = (char*)realloc(v->data, ncap);
    if (!p) return false;

    v->data = p;
    v->sz = ncap;
    return true;
}

bool sv_push(String *v, char c) {
    if (!v || !sv_reserve(v, v->len + 1)) return false;
    v->data[v->len++] = c;
    return true;
}

#endif

// one general string read-in
// it should handle stacl & heap stuff nicely 
// if something's len >= req -> should not alloc memory
// but if it's "too small" or 0 then it should malloc/realloc
// TODO : separate to 2 fn
void input_string(FILE *in, FILE *out, char **buf, size_t len, bool dynamic, const char *prompt) {
    if (prompt && out) {
        fputs(prompt, out);
        fflush(out);
    }
    if (!in || !len || !buf)
        return;

#if defined(DYN_STR) /* Dynamic-resize string */
    if (!dynamic)
        return;
    String *v;
    int c;
  read:;;
    v = sv_create_frm(*buf, len);
    if (!v) {
        fprintf(out, "malloc failed");
        goto free;
    }

    if ((c = fgetc(in)) != EOF) {
        do {
            if (c == '\n')
                goto nice;
            else if (c == '\r') {
                consume_crlf_tail(in);
                goto nice;
            }

            if (!sv_push(v, (char)c)) goto free;
        } while ((c = fgetc(in)) != EOF);
    } else { // EOF case
        handle_eof(in, out);
        fprintf(out, EOF_RETRY);
        *buf = sv_free_get(v);
        goto read;
    }
  nice:;;
    if (!sv_push(v, '\0')) goto free;
    *buf = sv_free_get(v);
    return;
  free:;;
    *buf = sv_free_get(v);
    return;

#else /* basic wrapper of fgets */
    char *b = *buf; // still ptr, so points to same val
    if (!b)
        return;
  read:;;
    if (!fgets(b, (int)len, in)) {
        if (feof(in))
            handle_eof(in, out);
        fprintf(out, EOF_RETRY);
        goto read; // retry only if we red nothing
    }
    size_t l = strlen(b);
    bool hadN = false;
    while (l > 0 && (b[l-1] == '\r' || b[l-1] == '\n')) {
        b[--l] = '\0';
        hadN = true;
    }
   
    // Check if we didn't get newline
    if (!hadN) {
        if (feof(in)) // EOF
            handle_eof(in, out);
        else // not EOF -> there is more on the input
            drain_line(in, out);
    }

    // optional - but only at heap buffers
  #if ZERO
    char *ptr;
    ptr = (char*)relloc(b, sizeof(char) * (l + 1));
    if (!ptr) {
        fprintf(out, "malloc failed");
        return;
    }
    *buf = ptr;
  #endif
    return;
#endif /* String getter types */

}

/*  separator  */

inp_res input_sep(FILE *in, int expected) {
    int c = fgetc(in);
    if (c == EOF) return INPUT_EOF;
    if (c == expected) return INPUT_OK;
    ungetc(c, in);
    return INPUT_SEP;
}

/*  utility  */

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

    int val;
    inp_res r;
read:
    r = input_int(IO, &val, min, max, text);

    if (input_drain_required(r))
        drain_line(IO);

    switch (r) {
        case INPUT_OK:
            return val;
        case INPUT_EOF:
            handle_eof(IO);
            printf(EOF_RETRY);
            goto read;
        case INPUT_RANGE:
            printf("input a number in [%d,%d]\n", min, max);
            goto read;
        default:
            printf("%s\n",input_status_str(r));
            goto read;
    }
}

uint inp_uint_s(const char* text, uint min, uint max) {
    if (min > max) { uint t = min; min = max; max = t; }

    uint val;
    inp_res r;
read:
    r = input_uint(IO, &val, min, max, text);

    if (input_drain_required(r))
        drain_line(IO);

    switch (r) {
        case INPUT_OK:
            return val;
        case INPUT_EOF:
            handle_eof(IO);
            printf(EOF_RETRY);
            goto read;
        case INPUT_RANGE:
            printf("input a number in [%u,%u]\n", min, max);
            goto read;
        default:
            printf("%s\n",input_status_str(r));
            goto read;
    }
}

double inp_double_s(const char* text, double min, double max) {
    if (min > max) { double t = min; min = max; max = t; }

    double val;
    inp_res r;
read:
    r = input_double(IO, &val, min, max, text);

    if (input_drain_required(r))
        drain_line(IO);

    switch (r) {
        case INPUT_OK:
            return val;
        case INPUT_EOF:
            handle_eof(IO);
            printf(EOF_RETRY);
            goto read;
        case INPUT_RANGE:
            printf("input a number in [%f,%f]\n", min, max);
            goto read;
        default:
            printf("%s\n",input_status_str(r));
            goto read;
    }
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
    input_string(stdin, stdout, &buf, BUF_LEN, true, text);
    return buf;
}
char *read_str(const char *msg, char *buf, size_t bufs) {
    input_string(stdin, stdout, &buf, bufs, false, msg);
    return buf;
}


/*
 * Reads one line from `FILE *in` (any length), including the trailing '\r' and '\n' if present.
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
    return fgetbatch(bufptr, n, in, true);
}
ptrdiff_t fgetfile(char **bufptr, size_t *n, FILE *in) {
    return fgetbatch(bufptr, n, in, false);
}
ptrdiff_t fgetbatch(char **bufptr, size_t *n, FILE *in, bool stpN) {
    if (!bufptr || !n || !in) {
        errno = EINVAL;
        return -1;
    }
    if (!*bufptr) *n = 0;

    size_t len = 0;
    int c;

    while ((c = fgetc(in)) != EOF) {
        /* need room for this char + the NUL terminator */
        if (len + 1 >= *n) {
            if (*n == 0) { /* if size is 0 */
                *n = BUF_LEN >> 1; // half size
            } else if (*n > (size_t)-1 / 2) { /* overflow guard */
                ungetc(c, in); /* restore consumed char */
                errno = ENOMEM;
                return -1;
            }
            size_t new_size = *n * 2;
            char *p = (char*)realloc(*bufptr, new_size); /* realloc is fine on "NULL" */
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

    if (len == 0) {
        /* Nothing read: either EOF (feof) or an error (ferror). */
        return -1;
    }

    (*bufptr)[len] = '\0';
    return (ptrdiff_t)len;
}




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

int parse_int_b(const char *text, int *num, int min, int max) {
    if (min > max) { int t = min; min = max; max = t; }
    int chars = parse_int(text, num);
    if (chars <= 0) return chars;
    if (*num < min || *num > max) return -1;
    return chars;
}

int parse_double_b(const char *text, double *num, double min, double max) {
    if (min > max) { double t = min; min = max; max = t; }
    int chars = parse_double(text, num);
    if (chars <= 0) return chars;
    if (isnan(*num)) return -1;
    if (*num < min || *num > max) return -1;
    return chars;
}

int parse_uint_b(const char *text, uint *num, uint min, uint max) {
    if (min > max) { uint t = min; min = max; max = t; }
    int chars = parse_uint(text, num);
    if (chars <= 0) return chars;
    if (*num < min || *num > max) return -1;
    return chars;
}

