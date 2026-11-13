
#ifndef ARR_OP_H
#define ARR_OP_H

#include <stdlib.h> // NULL
#include <stddef.h> // size_t

#if defined(MY_MEMCHECK)
#include "memcheck.h"
#endif

#if !defined(ARR_L) && !defined(ARR_S)
#  define ARR_L(x) x, ARR_S(x)
#  define ARR_S(x) sizeof(x)/sizeof(x[0])
#endif /* ARR_L and ARR_S */

#if defined(__GNUC__) || defined(__clang__)
#  define LAMBDA(ret, args, body) __extension__ ({ ret lambda_fn_ args body lambda_fn_; })
#endif
// this expands somethign like this :
// LAMBDA(void, (void *p), { printf("%d\n", *(int *)p); })
// to this :
// ({ void __fn__(void *p) { printf("%d\n", *(int *)p); } __fn__; })
//
// so nice ! - if GNUC not present panic at compile time !

// Returns max value, or 0
int arr_max(const int *arr, size_t len);
// Returns min value, or 0
int arr_min(const int *arr, size_t len);
// Returns sum, or 0
int arr_sum(const int *arr, size_t len);
// Fill with 0
void arr_fillN(int *arr, size_t len);
// Fill with random values 0..100
void arr_fillR(int *arr, size_t len);
// Fill with num
void arr_fill(int *arr, size_t len, int num);

// Bubble sort (ascending)
void arr_bubble(int *arr, size_t len);
// Reverse the array in place
void arr_rev(int *arr, size_t len);
// Multiply two arrays element-wise into mult
void arr_mul(const int *arr, const int *arr2, size_t len, int *mult);

// Print each index and value
void arr_print(const int *arr, size_t len);
// free-s the array's each element with the stdlib:free
void arr_free(void **arr, size_t len);
// same as above but with the input fn
void arr_free_fn(void **arr, size_t len, void (*free_fn)(void*));
// run a function for each item in the array
void arr_foreach(void **arr, size_t len, void (*fn)(void*));
// same as ^^ but with int
void arr_foreach_int(int *arr, size_t len, void (*fn)(int*));

#endif /* ARR_OP_H */

