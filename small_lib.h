#ifndef SMALL_LIB_H
#define SMALL_LIB_H

#include <stdbool.h>

/* Backards cmpatiblity ... MY dearest */
#include "input.h"

#ifndef UINT_TYPE
typedef unsigned int uint;
#endif /* UINT_TYPE */

// print the string + \n
void print(const char* str);
// clean the screen and move to 0,0
void clear(void);
// print to err + \n
void err(const char* str);

// compare, ++ = a bigger, -- = b bigger, 0 = equal
int cmp(int a, int b);
// generate a random between bounds
int rand_int(int min, int max);
// randm double (float) , 0-1
double rand_double(void);

#endif /* SMALL_LIB_H */

