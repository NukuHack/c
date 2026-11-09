#ifndef MY_UTILS
#define MY_UTILS

#include <stdio.h> // io
#include <stddef.h> // size_t as length

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

#endif /* MY_UTILS */

