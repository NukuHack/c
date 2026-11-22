#ifndef MY_UTILS
#define MY_UTILS

#include "my_macro.h"

#if ZERO
#  define __STDC_WANT_LIB_EXT1__ 1 // this adds "safe" methods from std
#endif

#define _POSIX_C_SOURCE 200809L
#include <stdio.h> // io
#include <stdbool.h> // bool
#include <stddef.h> // size_t, ptrdiff_t aka ssize_t
#include <stdlib.h> // basic memory & NULL
#include <string.h> // batch mem op
#include <stdint.h> // for max sizes
#include <limits.h> // limit for overflow checking
#include <ctype.h> // basic char methods
#include <errno.h> // error mainly for "EOF" and "errno" at builtin stuff

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#  if !defined(WIN)
#    define WIN
#  endif /* win-def */
#endif


/* 2 pointers in alpha order, returns NULL if eq */
char* alphabetic(char* a, char* b);
/* count the words (separator take in) */
int count_words(const char *str, const char *sep);
/* replace all old to upd */
void strrep(char *str, int len, char old, char upd);
/* shift argv, argc and return the shifted out variable */
char **shift_args(int *argc, char ***argv);
// print the string + \n
void print(const char* str);
// clean the screen and move to 0,0
void clear(void);
// print to err + \n
void err(const char* str);
// Sleeps for a given number of milliseconds
void slep(double millis);
// temp dir, not thread safe
const char *temp_dir(void);
/* small check */
bool check_file_exist(const char *name);
// compare, ++ = a bigger, -- = b bigger, 0 = equal
int cmp(int a, int b);
// same as above, just for pointers
int cmp_int_ptr(const void *a, const void *b);
// generate a random between bounds
int randi_b(int min, int max);
// random int
int randi(void);
// random double
double randd(void);
// random double, bounded
// please no NAN in this !!!
double randd_b(double min, double max);
/* ---- CPU time ---- */
double bench_cpu_now(void);



#if defined(MY_MEMCHECK)
#  undef MY_MEMCHECK
#  define MEMCHECK
#  include "memcheck.h"
#endif

#if ZERO
// this does not work
#  define LOGGING MY_LOGGING
#  undef MY_LOGGING
// this makes LOGGING defined to MY_LOGGING
// what does NOT exist at the time we need LOGGING
// so yeah -- "bad switch case" :
#elif defined(MY_LOGGING)
#  if MY_LOGGING == 0
#    define LOGGING 0
#  elif MY_LOGGING == 1
#    define LOGGING 1
#  elif MY_LOGGING == 2
#    define LOGGING 2
#  elif MY_LOGGING == 3
#    define LOGGING 3
#  elif MY_LOGGING == 4
#    define LOGGING 4
#  elif MY_LOGGING == 5
#    define LOGGING 5
#  elif MY_LOGGING == 6
#    define LOGGING 6
#  endif
#  undef MY_LOGGING
#  include "logging.h"
#endif


#endif /* MY_UTILS */

