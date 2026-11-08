// self
#include "input.h"
// impl
#include <limits.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>
#include "string.h"  /* for string input - dyn_memory */
#include "small_lib.h"   /* for IO */

/* helpers */

static void consume_crlf_tail(FILE *in) {
    int c = fgetc(in);
    if (c == EOF)
        return;
    if (c != '\n')
        ungetc(c, in);
}

static bool finish_number(FILE *in, int c) {
    if (c == EOF || c == '\n')
        return false;
    if (c == '\r') {
        consume_crlf_tail(in);
        return false;
    }
    ungetc(c, in);
    return true;
}

const char *input_status_str(inp_res s) {
    switch (s) {
        case INPUT_OK:       return "ok";
        case INPUT_EOF:      return "end of input";
        case INPUT_EMPTY:    return "empty";
        case INPUT_NAN:      return "not a number";
        case INPUT_OVERFLOW: return "overflow";
        case INPUT_STOPPED:  return "stopped at non-closing char";
        case INPUT_RANGE:    return "out of range";
        case INPUT_SEP:      return "separator not found";
    }
    return "unknown";
}

/* char-by-char read + parse -> no malloc */

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
            return INPUT_NAN;
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
    int overflow = 0;

    do {
        int d = c - '0';
        if (!overflow) {
            long long limit = (sign > 0)
                ? INT_MAX
                : (long long)INT_MAX + 1;

            if (n > (limit - d) / 10)
                overflow = 1;
            else
                n = n * 10 + d;
        }
    } while ((c = fgetc(in)) >= '0' && c <= '9');

    long long signed_n = n * sign;

    if (overflow || signed_n < min || signed_n > max) {
        if (c != EOF)
            ungetc(c, in);
        return overflow ? INPUT_OVERFLOW : INPUT_RANGE;
    }

    int stopped = finish_number(in, c);
    if (v) *v = (int)signed_n;
    return stopped ? INPUT_STOPPED : INPUT_OK;
}

inp_res input_uint(FILE *in, FILE *out, uint *v, uint min, uint max, const char *prompt) {
    if (min > max) { uint t = min; min = max; max = t; }

    if (prompt && out) { fputs(prompt, out); fflush(out); }

    int c = fgetc(in);
    if (c == EOF)
        return INPUT_EOF;
    if (c == '\n' || c == '\r') {
        if (c == '\r') consume_crlf_tail(in);
        return INPUT_EMPTY;
    }

    if (c == '+') {
        c = fgetc(in);
        if (c == EOF)
            return INPUT_NAN;
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
    int overflow = 0;

    do {
        uint d = (uint)(c - '0');
        if (!overflow) {
            if (n > (UINT_MAX - d) / 10u)
                overflow = 1;
            else
                n = n * 10u + d;
        }
    } while ((c = fgetc(in)) >= '0' && c <= '9');

    if (overflow || n < min || n > max) {
        if (c != EOF)
            ungetc(c, in);
        return overflow ? INPUT_OVERFLOW : INPUT_RANGE;
    }

    int stopped = finish_number(in, c);
    if (v) *v = n;
    return stopped ? INPUT_STOPPED : INPUT_OK;
}

inp_res input_double(FILE *in, FILE *out, double *v, double min, double max, const char *prompt) {
    if (min > max) { double t = min; min = max; max = t; }

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
            return INPUT_NAN;
        if (c == '\n' || c == '\r') {
            ungetc(c, in);
            return INPUT_NAN;
        }
        if (c < '0' && c != '.') {
            ungetc(c, in);
            return INPUT_NAN;
        }
    } else if (c < '0' && c != '.') {
        ungetc(c, in);
        return INPUT_NAN;
    }

    double int_part = 0.0;
    double frac_part = 0.0;
    double frac_scale = 1.0;
    int has_digit = 0;
    int frac_digits = 0;

    while (c >= '0' && c <= '9') {
        int_part = int_part * 10.0 + (c - '0');
        has_digit = 1;
        c = fgetc(in);
    }

    if (c == '.') {
        c = fgetc(in);
        while (c >= '0' && c <= '9') {
            if (frac_digits < 20) {
                frac_part = frac_part * 10.0 + (c - '0');
                frac_scale *= 10.0;
                frac_digits++;
            }
            has_digit = 1;
            c = fgetc(in);
        }
    }

    if (!has_digit) {
        if (c != EOF) ungetc(c, in);
        return INPUT_NAN;
    }

    double d = (int_part + frac_part / frac_scale) * sign;

    if (isinf(d) || d < min || d > max) {
        if (c != EOF) ungetc(c, in);
        return isinf(d) ? INPUT_OVERFLOW : INPUT_RANGE;
    }

    int stopped = finish_number(in, c);
    if (v) *v = d;
    return stopped ? INPUT_STOPPED : INPUT_OK;
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
    while ((c = fgetc(in)) != '\n' && c != '\r' && c != EOF)
        ;

    if (c == '\r') {
        int next = fgetc(in);
        if (next != '\n' && next != EOF)
            ungetc(next, in);
    }
}

void finp(void) {
    drain_line(stdin);
}

int inp_int(void) {
    return inp_int_s("input a num: ", 0, 2000);
}
double inp_double(void) {
    return inp_double_s("input a double: ", -INFINITY, INFINITY);
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

    while(true) {
        int val;
        inp_res r = input_int(stdin, stdout, &val, min, max, text);

        switch (r) {
            case INPUT_OK:
                return val;

            case INPUT_EOF:
                clearerr(stdin);
                print("\ntry again");
                continue;

            case INPUT_EMPTY:
                print("input a number");
                continue;

            case INPUT_NAN:
                print("input a number");
                drain_line(stdin);
                continue;

            case INPUT_OVERFLOW:
                print("number way too big");
                drain_line(stdin);
                continue;

            case INPUT_RANGE:
                printf("input a number in [%d,%d]\n", min, max);
                drain_line(stdin);
                continue;

            case INPUT_STOPPED:
                print("one single simple number (int)");
                drain_line(stdin);
                continue;

            case INPUT_SEP:
                drain_line(stdin);
                continue;
        }
    }
}

double inp_double_s(const char* text, double min, double max) {
    if (min > max) { double t = min; min = max; max = t; }

    while(true) {
        double val;
        inp_res r = input_double(stdin, stdout, &val, min, max, text);

        switch (r) {
            case INPUT_OK:
                return val;

            case INPUT_EOF:
                clearerr(stdin);
                print("\ntry again");
                continue;

            case INPUT_EMPTY:
                print("input a number");
                continue;

            case INPUT_NAN:
                print("input a real number");
                drain_line(stdin);
                continue;

            case INPUT_OVERFLOW:
                print("number way too big");
                drain_line(stdin);
                continue;

            case INPUT_RANGE:
                printf("input a number in [%f,%f]\n", min, max);
                drain_line(stdin);
                continue;

            case INPUT_STOPPED:
                print("one single simple number (double)");
                drain_line(stdin);
                continue;

            case INPUT_SEP:
                drain_line(stdin);
                continue;
        }
    }
}

char inp_char_s(const char* text) {
    while(true) {
        if (text) {
            fputs(text, stdout);
            fflush(stdout);
        }

        int ch = fgetc(stdin);
        if (ch == EOF) {
            clearerr(stdin);
            print("\ntry again");
            continue;
        }

        if (ch == '\n' || ch == '\r') {
            if (ch == '\r') consume_crlf_tail(stdin);
            print("input a char (char)");
            continue;
        }

        int c = fgetc(stdin);
        if (c == '\r') {
            consume_crlf_tail(stdin);
            c = '\n';
        }

        if (c != '\n' && c != EOF) {
            drain_line(stdin);
            print("input a char (char)");
            continue;
        }

        if (!is_alpha((char)ch)) {
            print("wrong char [a-zA-Z]");
            continue;
        }

        return (char)ch;
    }
}

char* inp_str_s(const char* text) {
    while(true) {
        if (text) {
            fputs(text, stdout);
            fflush(stdout);
        }

        String* v = sv_create();
        if (!v) return NULL;

        int got_any = 0;
        int c;

        while ((c = fgetc(stdin)) != EOF) {
            got_any = 1;

            if (c == '\n') {
                if (!sv_push(v, '\0')) { sv_free(v); return NULL; }
                return sv_free_get(v);
            }

            if (c == '\r') {
                consume_crlf_tail(stdin);
                if (!sv_push(v, '\0')) { sv_free(v); return NULL; }
                return sv_free_get(v);
            }

            if (!sv_push(v, (char)c)) { sv_free(v); return NULL; }
        }

        if (!got_any) {
            clearerr(stdin);
            print("\ntry again");
            sv_free(v);
            continue;
        }

        if (!sv_push(v, '\0')) { sv_free(v); return NULL; }
        return sv_free_get(v);
    }
}

/* parsers */
// since they already have the string in memory going char-by-char is bad, so just use the standard
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
    int chars = parse_int(text, num);
    if (chars <= 0) return chars;
    if (*num < min || *num > max) return -1;
    return chars;
}

int parse_double_b(const char *text, double *num, double min, double max) {
    int chars = parse_double(text, num);
    if (chars <= 0) return chars;
    if (isnan(*num)) return -1;
    if (*num < min || *num > max) return -1;
    return chars;
}

int parse_uint_b(const char *text, uint *num, uint min, uint max) {
    int chars = parse_uint(text, num);
    if (chars <= 0) return chars;
    if (*num < min || *num > max) return -1;
    return chars;
}

