#include "arr_op.h" // self
#include "my_utils.h"

#define LENGTH 20

int arr_main(void) {
    int tomb[LENGTH];

    arr_fill(ARR_L(tomb));
    arr_bubble(ARR_L(tomb));
    arr_reverse(int, tomb, ARR_S(tomb));

    arr_print(int, tomb, ARR_S(tomb));

    return 0;
}

int arr_max(const int *arr, size_t len) {
    if (!arr || len == 0)
        return 0;

    int num = arr[0];
    arr_foreach(int, var, arr, len)
        if (var > num) num = var;
    return num;
}
int arr_min(const int *arr, size_t len) {
    if (!arr || len == 0)
        return 0;

    int num = arr[0];
    arr_foreach(int, var, arr, len)
        if (var < num) num = var;
    return num;
}
int arr_sum(const int *arr, size_t len) {
    if (!arr || len == 0)
        return 0;

    int sum = 0;
    arr_foreach(int, var, arr, len)
        sum += var;
    return sum;
}


void arr_fill(int *arr, size_t len) {
    arr_foreach_p(int, var, arr, len)
        *var = rand_int(0, 100);
}

void arr_set(int *arr, size_t len, int num) {
    arr_foreach_p(int, var, arr, len)
        *var = num;
}

// todo : remove, use qsort
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
void arr_qsort(int *arr, size_t len) {
    if (!arr || len == 0)
        return;

    qsort(arr, len, sizeof(int), cmp_int_ptr);
}


