
#ifndef ARR_OP_H
#define ARR_OP_H

#include <stdlib.h> // NULL
#include "my_utils.h" // all utility

// "evaulate" everything only once
// no need for "too special" variable names since they are scoped
#define array_reverse(Ty, arr, len) \
    do { \
        size_t len_ = (size_t)(len); \
        Ty *ar_ = (arr); \
        if (ar_ != NULL && len_ >= 2u) \
        for (size_t i = 0; i < len_ / 2u; i++) { \
            Ty tmp = ar_[i]; \
            ar_[i] = ar_[len_ - i - 1u]; \
            ar_[len_ - i - 1u] = tmp; \
        } \
    } while (0)

// suffix the names, so less collision
#define arr_foreach(Ty, var, arr, len) \
    for (Ty var, *ar_s_ = (Ty *)(arr), *ar_p_ = ar_s_, \
         *ar_e_ = ar_s_ ? ar_s_ + (size_t)(len) : ar_s_; \
         ar_p_ != ar_e_ && ((var = *ar_p_), 1); ar_p_++)

/* Current index; valid only inside an arr_foreach body. */
#define arr_for_i ((size_t)(ar_p_ - ar_s_))

#define arr_foreach_p(Ty, var, arr, len) \
    for (Ty *var = (arr), *ar_e_ = var ? var + (size_t)(len) : var; var != ar_e_; ++var)

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
// Print each index and value
void arr_print(const int *arr, size_t len);
// Print each value, for strings 
void arr_print_str(char **arr, size_t len);

#endif /* ARR_OP_H */

