#include "arr_op.h" // self

#define LENGTH 20

int arr_main(void) {
    int tomb[LENGTH];

    arr_fill(ARR_L(tomb));
    arr_bubble(ARR_L(tomb));
    arr_rev(ARR_L(tomb));

    arr_print(ARR_L(tomb));

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
    arr_foreach_p(int, var, arr, len) {
        *var = rand_int(0, 100);
    }
}

void arr_set(int *arr, size_t len, int num) {
    arr_foreach_p(int, var, arr, len) {
        *var = num;
    }
}

// not even public, just ... for fun
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

void arr_rev(int *arr, size_t len) {
    array_reverse(int, arr, len);
}

void arr_print(const int *arr, size_t len) {
    arr_foreach(int, var, arr, len) {
        printf("idx %zu) : %d\n", arr_for_i, var);
    }
}

void arr_print_str(char **arr, size_t len) {
    for (size_t i = 0; i < len; i++)
        printf("idx %zu) : %s\n", i, arr[i]);
}

void arr_mul(const int *arr, const int *arr2, size_t len, int *mult) {
    if (!arr || !arr2 || !mult || len == 0)
        return;

    for (size_t i = 0; i < len; i++)
        mult[i] = arr[i] * arr2[i];
}


