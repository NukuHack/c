#ifndef MY_UTILS
#define MY_UTILS

#ifndef ZERO
// so i have sintax highlight in "if 0"
#  define ZERO 0
#endif /* ZERO */

#if ZERO
#  define __STDC_WANT_LIB_EXT1__ 1 // this adds "safe" methods from std
#endif

#include <stdio.h> // io
#include <stdbool.h> // bool
#include <stddef.h> // size_t, ptrdiff_t aka ssize_t
#include <stdlib.h> // basic memory & NULL
#include <string.h> // batch mem op

#if !defined(BUF_LEN)
#     define BUF_LEN 128
#endif /* BUF_LEN */

#if !defined(destroy)
#    define destroy(a) do { free(a);a=NULL; }while(0)
#endif /* destroy */

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#  if !defined(WIN)
#    define WIN
#  endif /* win-def */
#endif

#if !defined(ARR_L) && !defined(ARR_S)
#  define ARR_L(x) x, ARR_S(x)
#  define ARR_S(x) ((sizeof(x))/(sizeof(x[0])))
#endif /* ARR_L and ARR_S */

#if !defined(CLAMP)
#  define CLAMP(a, b, ty) (ty)MIN((a),(b)) // clamp a to b, return min, in type ty
#endif

#if !defined(MIN) && !defined(MAX) && !defined(ABS)
#  define MIN(a, b) ((a) < (b) ? (a) : (b))
#  define MAX(a, b) ((a) > (b) ? (a) : (b))
#  define ABS(a) ((a) < 0 ? -(a) : (a))
#endif /* MIN-MAX-ABS */

#if !defined(LLU) && !defined(LL)
#  define LLU long long unsigned int
#  define LL long long int
#endif /* LLU-LL */

#if !defined(UINT_TYPE) && !defined(uint)
#  define UINT_TYPE
   typedef unsigned int uint;
#  define uint uint
#endif /* UINT_TYPE */

#if !defined(KB) && !defined(MB) && !defined(GB) // rest not listed
extern const char *const units[]; // in c
// yes i know they "should be" called like "kibibyte" for 1024 multipliers
// but usually they are called "kilobyte" even if they should be just powers of 10 
// #define B 1       //  byte      2**0
#  define KB 1024    // kilobyte   2**10
#  define MB KB * KB // megabyte   2**20
#  define GB MB * KB // gigabyte   2**30
#  define TB GB * KB // terrabyte  2**40
#  define PB TB * KB // petabyte   2**50
#  define EB PB * KB // exabyte    2**60
#  define ZB EB * KB // zettabyte  2**70
#  define YB ZB * KB // yottabyte  2**80
#  define RB YB * KB // ronnabyte  2**90
#  define QB RB * KB // qettabyte  2**100
#endif


/* swap the data of 2 pointers */
void mem_swap(void *a, void *b, size_t size);
/* swap the values of 2 int */
void swap(int *a, int *b);
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
// print a single int + \n
void printi(int num);
// print a single new-line
void printn(void);
// clean the screen and move to 0,0
void clear(void);
// print to err + \n
void err(const char* str);
// Sleeps for a given number of milliseconds
void slep(double millis);
// temp dir, caller has to free it
const char *temp_dir(void);
/* small check */
bool check_file_exist(const char *name);

// compare, ++ = a bigger, -- = b bigger, 0 = equal
int cmp(int a, int b);
// same as above, just for pointers
int cmp_int_ptr(const void *a, const void *b);
/* compare 2 int ptr (value), return bigger (a if eq) */
int *cmpp(int *a, int *b);
// generate a random between bounds
int rand_int(int min, int max);
// randm double (float) , 0-1
double rand_double(void);
/* ---- CPU time ---- */
double bench_cpu_now(void);



#if defined(MY_MEMCHECK)
#  undef MY_MEMCHECK
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
#  if   MY_LOGGING == 0
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

