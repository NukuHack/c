#ifndef MY_UTILS
#define MY_UTILS

#include <stdio.h> // io

/* swap the values of 2 pinters */
void swap(int *a, int *b);
/* compare 2 int ptr (value), return bigger (a if eq) */
int *cmpp(int *a, int *b);

// print the string + \n
void print(const char* str);
// print a single num + \n
void printi(int num);
// clean the screen and move to 0,0
void clear(void);
// print to err + \n
void err(const char* str);
// Sleeps for a given number of milliseconds
void slep(double millis);

// compare, ++ = a bigger, -- = b bigger, 0 = equal
int cmp(int a, int b);
// generate a random between bounds
int rand_int(int min, int max);
// randm double (float) , 0-1
double rand_double(void);

#endif /* MY_UTILS */

