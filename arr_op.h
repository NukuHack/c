
#ifndef ARR_OP_H
#define ARR_OP_H

// Returns max value, or 0
int arr_max(const int *arr, int len);
// Returns min value, or 0
int arr_min(const int *arr, int len);
// Returns sum, or 0
int arr_sum(const int *arr, int len);
// Fill with 0
void arr_fillN(int *arr, int len);
// Fill with random values
void arr_fillR(int *arr, int len);
// Fill with num
void arr_fill(int *arr, int len, int num);

// Bubble sort (ascending)
void arr_bubble(int *arr, int len);
// Print each index and value
void arr_print(const int *arr, int len);
// Reverse the array in place
void arr_reverse(int *arr, int len);
// Multiply two arrays element-wise into mult
void arr_mul(const int *arr, const int *arr2, int len, int *mult);

#endif /* ARR_OP_H */

