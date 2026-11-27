#ifndef MY_IO_H
#define MY_IO_H

#include "my_utils.h" // all utility

/*
 * get:
 * reads from "FILE *in", including '\n' (even in stpN mode)
 * The buffer pointed to by "*bufptr" is alloc/realloc -ed
 * and is always "\0"-terminated on success
 *
 * put:
 * writes to "FILE *out", including '\n' (even in stpN mode)
 * The buffer and size is not touched, writes up to *n chars
 *
 * RETURN :
 * On success : returns number of bytes read/written (excluding the "\0")
 * On error   : returns -1 and sets errno
 * On instant EOF in read *wo "stpN" : returns -1 with no data consumed
 * - make sure "EOF" is only retried for streams that support it, and will not just loop forever
 *
 * MEMORY :
 * The caller owns "*bufptr" and must free() it
 *
 * in getbatch "*bufptr" must be "NULL", or a pointer from malloc/realloc (with (atleast) *n size)
 *  - if it is stack allocated resize is "Undefined" best case is crash use "fgetstack" for that
 *
 * in putbatch "*bufptr" can be anything other than "NULL" (with (atleast) *n size).
 */
ptrdiff_t fgetbatch(char **bufptr, size_t *n, FILE *in, bool stpN);
ptrdiff_t fgetstack(char **bufptr, size_t *n, FILE *in);
ptrdiff_t fputbatch(char **bufptr, size_t *n, FILE *out, bool stpN);
// put stack is not needed, since we can write from stack witht the same buffer

/* "high-level" input functins */
// the default behaviour of these wrappers are just to get a single line from stdin 
// output to stdout if incorrect, and re-try 

// used for char, allows stuff that returns "true"
typedef int (predicate)(int);

// Num inputs have a min-max of 10K, reason being ... nothing
// usually we take in that kind of numbers, over 10k the caller should be explicit
int inpi(void);
uint inpu(void);
double inpd(void);
char inpc(void); // only accepts alphabetic chars
bool confirm(void); // returns true on 'y' || 'Y', retry onanything not 'n' || 'N'
// bounded versions 
int inpi_s(const char* text, int min, int max);
uint inpu_s(const char* text, uint min, uint max);
double inpd_s(const char* text, double min, double max);
char inpc_s(const char* text, predicate *filter);

// string stuff - MALLOCS must free it after use 
// read_str does not malloc, it uses the input buffer 
// all consumes any extra from stdin, No re-try
// on fail it return "NULL" (ofc no mem loss)
char *inps(void);
// TODO : max length for this
char *inps_s(const char* text);
char *read_str(const char *msg, char* buf, size_t bufs);

/* parse from string, return the consumed chars */
int parse_i(const char* text, int* num);
int parse_d(const char* text, double* num);
int parse_u(const char* text, uint* num);
/* small helpers for parse */

#define try_parse(ty, txt, num) \
    do { \
        char *text = (txt); \
        ty *num_ = (num); \
        size_t len = strlen(text); \
        bool ret = len != 0 && len == (size_t)parse_##ty(text, num_); \
        return ret; /* how thooo */ \
    } while(0)

bool try_parse_i(const char *txt, int *num);
bool try_parse_u(const char *txt, uint *num);
bool try_parse_d(const char *txt, double *num);

#endif /* MY_IO_H */

