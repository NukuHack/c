
#ifndef MY_PARSE_H
#define MY_PARSE_H

#define NPE -1 // short for NullPointer
#define OVER -2 // short for overflow
#define OOB -3 // short for OutOfBounds
#define NOP 0 // could not parse (0 chars consumed, might not be bad)

typedef unsigned uint;

// parse from string, return the consumed chars
// since it has return type, the "real" input has to be set through the function inputs
// -- so dear pointers yet again --
int parse_int(char* text, int* num);
int parse_double(char* text, double* num);
int parse_uint(char* text, uint* num);

int parse_int_b(char* text, int* num, int min, int max);
int parse_double_b(char* text, double* num, double min, double max);
int parse_uint_b(char* text, uint* num, uint min, uint max);

#endif /* MY_PARSE_H */


