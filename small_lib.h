
#ifndef SMALL_LIB_H
#define SMALL_LIB_H

#include <stdbool.h>

// print the string + \n
void print(const char* str);
// clean the screen and move to 0,0
void clear(void);
// print to err + \n
void err(const char* str);
// flush input
void finp(void);

// input a char
char inp_char(void);
// input a char, custom prompt
char inp_char_s(const char* text);
// input a string
char* inp_str(void);
// input a string, custom prompt
char* inp_str_s(const char* text);
// input an int
int inp_int(void);
// input an int, custom prompt and bounds
int inp_int_s(const char* text, int min, int max);
// input a double
double inp_double(void);
// input a double. custom prompt and bounds
double inp_double_s(const char* text, double min, double max);

// compare, ++ = a bigger, -- = b bigger, 0 = equal
int cmp(int a, int b);
// generate a random between bounds
int rand_int(int min, int max);
// randm double (float) , 0-1
double rand_double(void);


#endif /* SMALL_LIB_H */

