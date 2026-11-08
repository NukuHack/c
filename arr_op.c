#include "arr_op.h"
#include <stdio.h>
#include "small_lib.h"

#define LENGTH 50

int arr_main(void) {
    int tomb[LENGTH];
    int *arr = tomb;

    arr_fillR(arr, LENGTH);
    arr_bubble(arr, LENGTH);
    arr_reverse(arr, LENGTH);

    arr_print(arr, LENGTH);

    return 0;
}

int arr_max(const int *arr, int len) {
    if (!arr || len <= 0)
        return 0;

    int num = arr[0];
    for (int i = 1; i < len; i++) {
        if (arr[i] > num)
            num = arr[i];
    }
    return num;
}

int arr_min(const int *arr, int len) {
    if (!arr || len <= 0)
        return 0;

    int num = arr[0];
    for (int i = 1; i < len; i++) {
        if (arr[i] < num)
            num = arr[i];
    }
    return num;
}

int arr_sum(const int *arr, int len) {
    if (!arr || len <= 0)
        return 0;

    int total = 0;
    for (int i = 0; i < len; i++) {
        total += arr[i];
    }
    return total;
}

void arr_fillN(int *arr, int len) {
    arr_fill(arr, len, 0);
}

void arr_fillR(int *arr, int len) {
    if (!arr || len <= 0)
        return;

    for (int i = 0; i < len; i++) {
        arr[i] = rand_int(0, 100);
    }
}

void arr_fill(int *arr, int len, int num) {
    if (!arr || len <= 0)
        return;

    for (int i = 0; i < len; i++) {
        arr[i] = num;
    }
}

void arr_bubble(int *arr, int len) {
    if (!arr || len <= 0)
        return;

    for (int i = 0; i < len - 1; i++) {
        for (int j = 0; j < len - 1 - i; j++) {
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }
}

void arr_reverse(int *arr, int len) {
    if (!arr || len <= 0)
        return;

    for (int i = 0; i < len / 2; i++) {
        int tmp = arr[i];
        arr[i] = arr[len - i - 1];
        arr[len - i - 1] = tmp;
    }
}

void arr_print(const int *arr, int len) {
    if (!arr || len <= 0)
        return;

    for (int i = 0; i < len; i++) {
        printf("idx:%d : %d\n", i, arr[i]);
    }
}

void arr_mul(const int *arr, const int *arr2, int len, int *mult) {
    if (!arr || !arr2 || !mult || len <= 0)
        return;

    for (int i = 0; i < len; i++) {
        mult[i] = arr[i] * arr2[i];
    }
}


