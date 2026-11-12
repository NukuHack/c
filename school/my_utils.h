#ifndef MY_UTILS
#define MY_UTILS

#if 0
#  define __STDC_WANT_LIB_EXT1__ 1 // this adds "safe" methods from std
#endif

#include <stdio.h> // io
#include <stdbool.h> // bool
#include <stddef.h> // size_t as length

#define ARR_L(x) x, ARR_S(x)
#define ARR_S(x) sizeof(x)/sizeof(x[0])

#ifdef __NOT_DEFINED
// so i have sintax highlight in "if 0"
#undef __NOT_DEFINED
#endif /* __NOT_DEFINED */

#ifndef LLU
#  define LLU long long unsigned int
#endif /* LLU */

/* swap the data of 2 pointers */
void mem_swap(void *a, void *b, size_t size);
/* swap the values of 2 pinters */
void swap(int *a, int *b);
/* compare 2 int ptr (value), return bigger (a if eq) */
int *cmpp(int *a, int *b);
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
// print a single num + \n
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
char *temp_dir(void);

// compare, ++ = a bigger, -- = b bigger, 0 = equal
int cmp(int a, int b);
// generate a random between bounds
int rand_int(int min, int max);
// randm double (float) , 0-1
double rand_double(void);


/* FILE stuff */
bool check_file_exist(const char *name);


#endif /* MY_UTILS */

