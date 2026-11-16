#ifndef INPUT_H
#define INPUT_H

#include "my_utils.h" // all utility

// used for char, allows stuff that returns true
typedef int (predicate)(int);

/*
 * Reads one line from "FILE *in", including the trailing "new-line" if present.
 * The buffer pointed to by "*bufptr" is allocated/reallocated as needed and is
 * always "\0"-terminated on success.
 *
 * On success : returns number of bytes read (excluding the "\0").
 * On (1) EOF : returns -1 with no data consumed; errno unchanged.
 *   - if we have have "bool stop-newline" false, ofc we get EOF ...
 * On error   : returns -1 and sets errno.
 *
 * The caller owns "*bufptr" and must free() it (possible "NULL").
 * "*bufptr" must be "NULL", or a pointer from malloc/realloc (with (atleast) *n size).
 * If it is stack allocated resize is "Undefined" best case is crash.
 *   - use "fgetstack" for that
 */
ptrdiff_t fgetbatch(char **bufptr, size_t *n, FILE *in, bool stpN);
ptrdiff_t fgetstack(char **bufptr, size_t *n, FILE *in);

/* "high-level" input functins */
// the default behaviour of these wrappers are just to get a single thing from stdin output to stdout if incorrect
// and re-try; make sure "EOF" is only retried for streams that support it, and will not just loop forever

int inpi(void);
uint inpu(void);
double inpd(void);
char inpc(void);
char* inps(void);

int inpi_s(const char* text, int min, int max);
uint inpu_s(const char* text, uint min, uint max);
double inpd_s(const char* text, double min, double max);
char inpc_s(const char* text, predicate *filter);
char* inps_s(const char* text);
/* read to buff, like fgets, return buffptr if success, "NULL" otherwise */
char *read_str(const char *msg, char* buf, size_t bufs);

// parse from string, return the consumed chars
int parse_i(const char* text, int* num);
int parse_d(const char* text, double* num);
int parse_u(const char* text, uint* num);
#endif /* INPUT_H */

