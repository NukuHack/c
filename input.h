#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include <stdbool.h>

#ifndef UINT_TYPE
typedef unsigned int uint;
#endif /* UINT_TYPE */

/* input result */
typedef enum {
    INPUT_OK = 0,      /* value read; \n / \r / EOF ended it */
    INPUT_EOF,         /* EOF before anything */
    INPUT_EMPTY,       /* first char was \n or \r (consumed) */
    INPUT_NAN,         /* first char not part of the number (pushed back) */
    INPUT_OVERFLOW,    /* doesn't fit in the target type */
    INPUT_STOPPED,     /* parsed a value; stopped at a non-closing char */
    INPUT_RANGE,       /* outside [min, max] */
    INPUT_SEP          /* input_sep: expected char not found */
} inp_res;

const char *input_status_str(inp_res s);

static inline bool input_retry(inp_res s) {
    return s == INPUT_EMPTY || s == INPUT_NAN;
}
static inline bool input_rejected(inp_res s) {
    return s == INPUT_OVERFLOW || s == INPUT_RANGE || s == INPUT_SEP || s == INPUT_STOPPED;
}
static inline bool input_drain_required(inp_res s) {
    return s == INPUT_NAN || s == INPUT_STOPPED || s == INPUT_OVERFLOW || s == INPUT_RANGE || s == INPUT_SEP;
}

/* low-level FILE parsers */
inp_res input_int(FILE *in, FILE *out, int *v, int min, int max, const char *prompt);

inp_res input_uint(FILE *in, FILE *out, uint *v, uint min, uint max, const char *prompt);

inp_res input_double(FILE *in, FILE *out, double *v, double min, double max, const char *prompt);

/* Consume exactly `expected`; otherwise leave the char in the stream. */
inp_res input_sep(FILE *in, int expected);

/* Throw away the rest of the current line. */
void drain_line(FILE *in);

/* "high-level" input functins */
// the default behaviour of these wrappers are just to get a single thing from stdin output to stdout if incorrect
// and re-try; make sure EOF is only retried for streams that support it, and will not just loop forever

void finp(void);

char inp_char(void);
char* inp_str(void);
int inp_int(void);
double inp_double(void);

char inp_char_s(const char* text);
char* inp_str_s(const char* text);
int inp_int_s(const char* text, int min, int max);
double inp_double_s(const char* text, double min, double max);

/* Backards cmpatiblity ... MY dearest */

// parse from string, return the consumed chars
int parse_int(const char* text, int* num);
int parse_double(const char* text, double* num);
int parse_uint(const char* text, uint* num);

int parse_int_b(const char* text, int* num, int min, int max);
int parse_double_b(const char* text, double* num, double min, double max);
int parse_uint_b(const char* text, uint* num, uint min, uint max);

#endif /* INPUT_H */

