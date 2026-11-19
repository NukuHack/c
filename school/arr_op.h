
#ifndef ARR_OP_H
#define ARR_OP_H

#include <stdlib.h> // NULL
#include "my_utils.h" // all utility

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

// Bubble sort (ascending)
void arr_bubble(int *arr, size_t len);
// qsort, it is just a wrapper for the std's impl
void arr_qsort(int *arr, size_t len);
// Reverse the array in place
void arr_rev(int *arr, size_t len);
// Multiply two arrays element-wise into mult
void arr_mul(const int *arr, const int *arr2, size_t len, int *mult);

#endif /* ARR_OP_H */

