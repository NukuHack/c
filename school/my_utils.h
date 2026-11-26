#ifndef MY_UTILS
#define MY_UTILS

#if defined(__GNUC__) && !defined(__clang__)
// prevents hyper optimized loops to use std methods (like memset)
#  pragma GCC push_options
#  pragma GCC optimize("no-tree-loop-distribute-patterns")
#endif

#include "my_macro.h"

#include <limits.h> // limit for overflow checking
#include <ctype.h> // basic char methods

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#  if !defined(WIN)
#    define WIN
#  endif /* win-def */
#endif

// generate a random between bounds
int randi_b(int min, int max);
// random int
int randi(void);
// random double
double randd(void);
// random double, bounded; please no NAN in this !!!
double randd_b(double min, double max);
/* 2 pointers in alpha order, returns NULL if eq */
char* alphabetic(char* a, char* b);
/* count the words (separator take in) */
int count_words(const char *str, const char *sep);
/* replace all old to upd */
void strrep(char *str, int len, char old, char upd);
/* shift argv, argc and return the shifted out variable */
char **shift_args(int *argc, char ***argv);
// clean the screen and move to 0,0
void clear(void);
// Sleeps for a given number of milliseconds
void slep(double millis);
// compare, ++ = a bigger, -- = b bigger, 0 = equal
int cmp(int a, int b);
// same ^ just with pointers
int cmp_int_ptr(const void *a, const void *b);

// temp dir, not thread safe
const char *temp_dir(void);
/* small check */
bool check_file_exist(const char *name);
/* ---- CPU time ---- */
double bench_cpu_now(void);

// Merged Array_Operations into this

// Returns max value, or 0
int arr_max(const int *arr, size_t len);
// Returns min value, or 0
int arr_min(const int *arr, size_t len);
// Returns sum, or 0
int arr_sum(const int *arr, size_t len);
// Fill with random values 0..100
void arr_fill(int *arr, size_t len);
// Fill with num
void arr_set(int *arr, size_t len, int num);
// qsort, it is just a wrapper for the std's impl
void arr_qsort(int *arr, size_t len);



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

