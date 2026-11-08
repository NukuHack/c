#include "input.h" // self
#include <limits.h> // limit for overflow checking
#include <math.h> // double stuff for overflow & NAN
#include <errno.h> // error checking mainly "EOF" and "strto_()" builtin stuff
#include "my_utils.h" // basic io
#include <ctype.h> // basic char methods
#include <stdlib.h> // basic memory & NULL
#include <string.h> // batch mem op

#if !defined(DYN_STR) /* DYN_STR */
#  ifndef BUF_LEN
#    define BUF_LEN 100
#  endif
#else
#  include "string.h" // dyn-mem & struct
#endif /* DYN_STR */

#define IO stdin,stdout
#define destroy(a) do { free(a);a=NULL; }while(0)

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
    return !(s == INPUT_OK || s == INPUT_EOF);
}
bool input_drain_required(inp_res s) {
    switch (s) {
        default:
        case INPUT_EOF:
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
        case INPUT_EMPTY: return "empty"; //  \n  -  \r\n
        case INPUT_RANGE: return "out of range"; // [10;40] - 300
        case INPUT_STOPPED: return "stopped (partially usable input)"; // 1234a
        case INPUT_OVERFLOW: return "overflow"; // more than 2^64 ...
    }
    return "unknown";
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

void input_char(FILE *in, FILE *out, char *v, bool(*filter)(int), const char *prompt) {
    int c, ch;
read:
    if (prompt && out) { fputs(prompt, out); fflush(out); }

    ch = fgetc(in);
    if (ch == EOF) {
        handle_eof(in, out);
        print("try again");
        goto read;
    }

    if (ch == '\n' || ch == '\r') {
        if (ch == '\r') consume_crlf_tail(in);
        print("input a char (char)");
        goto read;
    }

    c = fgetc(in);
    if (c == '\r') {
        consume_crlf_tail(in);
        c = '\n';
    }

    if (c != '\n' && c != EOF) { // EOF case
        drain_line(in);
        print("input a char (char)");
        goto read;
    }

    if (filter && !filter(ch)) {
        print("wrong char [not allowed]");
        goto read;
    }

    *v = (char)ch;
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

void drain_line(FILE *in) {
    int c;
    while ((c = fgetc(in)) != '\n' && c != '\r' && c != EOF);

    if (c == '\r' && (c = fgetc(in)) != '\n' && c != EOF)
        ungetc(c, in);

    /* we ran out of input before any line terminator -> the user's
       cursor is still sitting at the end of the current line, so
       emit one ourselves to keep prompts/errors on their own line */
    if (c == EOF) {
        handle_eof(in, stdout);
    }
}

// high level inputs have a min-max of 10K, reason being ... nothing
// usually we take in that kind of numbers, if a number is over 10k the caller should be explicit

int inp_int(void) {
    return inp_int_s("input an int: ", -10000, 10000);
}
uint inp_uint(void) {
    // TODO : make a real impl.
    // reject negative numbers
    return (uint)inp_int_s("input a uint: ", 0, 10000);
}
double inp_double(void) {
    return inp_double_s("input a double: ", -10000, 10000);
}
char inp_char(void) {
    return inp_char_s("input a char: ");
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
    r = input_int(stdin, stdout, &val, min, max, text);

    if (input_drain_required(r))
        drain_line(stdin);

    switch (r) {
        case INPUT_OK:
            return val;
        case INPUT_EOF:
            handle_eof(IO);
            print("closed input, but retry");
            goto read;
        case INPUT_RANGE:
            printf("input a number in [%d,%d]\n", min, max);
            goto read;
        default:
            print(input_status_str(r));
            goto read;
    }
}

double inp_double_s(const char* text, double min, double max) {
    if (min > max) { double t = min; min = max; max = t; }

    double val;
    inp_res r;
read:
    r = input_double(stdin, stdout, &val, min, max, text);

    if (input_drain_required(r))
        drain_line(stdin);

    switch (r) {
        case INPUT_OK:
            return val;
        case INPUT_EOF:
            handle_eof(IO);
            print("closed input, but retry");
            goto read;
        case INPUT_RANGE:
            printf("input a number in [%f,%f]\n", min, max);
            goto read;
        default:
            print(input_status_str(r));
            goto read;
    }
}

char inp_char_s(const char* text) {
    char c;
    input_char(stdin, stdout, &c, isalpha, text);
    return c;
}

#if defined(DYN_STR) /* DYN_STR */
char *inp_str_s(const char *text) {
    String *v;
    int got_any, c;
read:
    if (text) {
        fputs(text, stdout);
        fflush(stdout);
    }

    v = sv_create();
    if (!v) goto free;

    got_any = 0;

    while ((c = fgetc(stdin)) != EOF) {
        got_any = 1;

        if (c == '\n') {
            if (!sv_push(v, '\0')) goto free;
            return sv_free_get(v);
        } else if (c == '\r') {
            consume_crlf_tail(stdin);
            if (!sv_push(v, '\0')) goto free;
            return sv_free_get(v);
        }

        if (!sv_push(v, (char)c)) goto free;
    }

    if (!got_any) { // EOF case
        handle_eof(IO);
        print("try again");
        sv_free(v);
        goto read;
    }

    if (!sv_push(v, '\0')) goto free;
    return sv_free_get(v);
free:
    sv_free(v);
    return NULL;
}
#elif defined(FGETS_STR)
char *inp_str_s(const char *text) {
    char tmp[BUF_LEN], *str;
    int red;
    bool hadN;
read:
    if (text) {
        fputs(text, stdout);
        fflush(stdout);
    }

    if (fgets(tmp, BUF_LEN, stdin) == NULL) {
        if (feof(stdin))
            handle_eof(IO);
        print("try again");
        goto read;
    }
    // here tmp is nicely set
    red = strlen(tmp);
    hadN = false;
    while (red > 0 && tmp[red-1] == '\n') {
        // Fgets reads the new line as char, and puts it in the buffer
        // here I don't want that, so manual removal
        // TODO: we should also handle windows \r\n
        tmp[--red] = '\0';
        hadN = true;
    }
   
    // Check if we read without a newline
    if (red > 0 && !hadN) {
        if (feof(stdin)) // EOF
            handle_eof(IO);
        else // not EOF -> there is more on the input
            drain_line(stdin);
    }

    str = (char*)malloc(sizeof(char) * (red + 1));
    if (!str) {
        err("malloc failed");
        return NULL;
    }
    memcpy(str, tmp, red);
    str[red] = '\0';
    return str;
}
#else /* non DYN and non FGETS */
// old clunky, but finally works with EOF
char* inp_str_s(const char* text) {
    char tmp[BUF_LEN], *str;
    int red, c;
read:
    if (text) {
        fputs(text, stdout);
        fflush(stdout);
    }
    c = getchar();

    if (c == EOF) {
        handle_eof(IO);
        print("try again");
        goto read;
    } else if (c == '\n')
        goto memalloc; // we do allow empty

    tmp[0] = (char)c;
    red = 1;

    while ((c = getchar()) != '\n' && red < BUF_LEN) {
        /* EOF after a partial line: accept what we got */
        if (c == EOF) {
            handle_eof(IO);
            break;
        }
        tmp[red++] = (char)c;
    }
    if (red == BUF_LEN)
        drain_line(stdin);
memalloc:
    str = (char*)malloc(sizeof(char) * (red + 1));
    if (!str) {
        err("malloc failed");
        return NULL;
    }
    memcpy(str, tmp, red);
    str[red] = '\0';
    return str;
}
#endif /* DYN_STR */


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

