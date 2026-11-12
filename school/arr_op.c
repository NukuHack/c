#include "arr_op.h" // self
#include <stdio.h> // printf
#include "my_utils.h" // for random

#define LENGTH 20

#if !defined(LAMBDA)
void lambda_random(int *p) {
    *p = rand_int(0, 100);
}
void lambda_print(int *p) {
    printf("idx:%zu : %d\n", (size_t)(p-arr), *p);
}
#endif /* Lambda */

int arr_main(void) {
    int tomb[LENGTH];

    arr_fillR(ARR_L(tomb));
    arr_bubble(ARR_L(tomb));
    arr_rev(ARR_L(tomb));

    arr_print(ARR_L(tomb));

    return 0;
}

int arr_max(const int *arr, size_t len) {
    if (!arr || len == 0)
        return 0;

    int num = arr[0];
    for (size_t i = 1; i < len; i++)
        if (arr[i] > num)
            num = arr[i];
    return num;
}

int arr_min(const int *arr, size_t len) {
    if (!arr || len == 0)
        return 0;

    int num = arr[0];
    for (size_t i = 1; i < len; i++)
        if (arr[i] < num)
            num = arr[i];
    return num;
}

int arr_sum(const int *arr, size_t len) {
    if (!arr || len == 0)
        return 0;

    int sum;
    for (sum = 0; len > 0; len--)
        sum += *(arr++);
    return sum;
}

void arr_fillN(int *arr, size_t len) {
    arr_fill(arr, len, 0);
}

void arr_fillR(int *arr, size_t len) {
    arr_foreach_int(arr, len,
#if defined(LAMBDA)
    LAMBDA(void, (int *p), { *p = rand_int(0, 100); })
#else
    lambda_random
#endif
        );
}

void arr_fill(int *arr, size_t len, int num) {
#if defined(LAMBDA)
    arr_foreach_int(arr, len,
        LAMBDA(void, (int *p), { *p = num; })
        );
#else
    // this needs 2 input, but the 2nd is not visible from outside
    if (!arr || !fn || len == 0)
        return;
    for (size_t i = 0; i < len; i++)
        arr[i] = num;
#endif
}

// not even public, just ... for fun
void bubbly(int *arr, size_t len) {
    for (size_t i = 0; i < len - 1; i++)
        for (size_t j = 0; j < len - 1 - i; j++)
            if (arr+j == cmpp(arr + j, arr + j + 1))
                swap(arr + j, arr + j + 1);
}

void arr_bubble(int *arr, size_t len) {
    if (!arr || len == 0)
        return;

    for (size_t i = 0; i < len - 1; i++)
        for (size_t j = 0; j < len - 1 - i; j++)
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
}

// "evaulate" everything only once
#define array_reverse(Ty, arr, len)                                           \
do {                                                                          \
    size_t ar_rev_n_ = (size_t)(len);                                         \
    Ty *ar_rev_a_ = (arr);                                                    \
    if (ar_rev_a_ != NULL && ar_rev_n_ >= 2u)                                 \
        for (size_t ar_rev_i_ = 0; ar_rev_i_ < ar_rev_n_ / 2u; ar_rev_i_++) { \
            Ty ar_rev_t_ = ar_rev_a_[ar_rev_i_];                              \
            ar_rev_a_[ar_rev_i_] = ar_rev_a_[ar_rev_n_ - ar_rev_i_ - 1u];     \
            ar_rev_a_[ar_rev_n_ - ar_rev_i_ - 1u] = ar_rev_t_;                \
        }                                                                     \
} while (0)

void arr_rev(int *arr, size_t len) {
    array_reverse(int, arr, len);
}

void arr_foreach(void **arr, size_t len, void (*fn)(void*)) {
    if (!arr || !fn || len == 0)
        return;
    for (size_t i = 0; i < len; i++)
        fn(arr[i]);
}
void arr_foreach_int(int *arr, size_t len, void (*fn)(int*)) {
    if (!arr || !fn || len == 0)
        return;
    for (size_t i = 0; i < len; i++)
        fn(&arr[i]);
}

void arr_print(const int *arr, size_t len) {
    arr_foreach_int((int*)arr, len,
#if defined(LAMBDA)
    LAMBDA(void, (int *p), { printf("idx:%zu : %d\n", (size_t)(p-arr), *p); })
#else
    lambda_print
#endif
        );
}

void arr_mul(const int *arr, const int *arr2, size_t len, int *mult) {
    if (!arr || !arr2 || !mult || len == 0)
        return;

    for (size_t i = 0; i < len; i++)
        mult[i] = arr[i] * arr2[i];
}

void arr_free(void **arr, size_t len) {
    arr_foreach(arr, len,
#if defined(LAMBDA)
    LAMBDA(void, (void *p), { free(p); })
#else
    free
#endif
        );
}
void arr_free_fn(void **arr, size_t len, void (*free_fn)(void*)) {
    arr_foreach(arr, len,
#if defined(LAMBDA)
    LAMBDA(void, (void *p), { free_fn(p); })
#else
    free_fn
#endif
        );
}

