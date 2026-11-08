#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
extern "C" {
#include "small_lib.h"
#include "input.h"
#include "list.h"
}

int main(void) {
    bool riad = false;
    int n = inp_int_s(NULL, 1, 100);

    List* szenny = list_make_sz(n);
    if (!szenny)
        return 1;

    List* riado = list_make();
    if (!riado)
        return 1;

    for (int i = 0; i<n; i++) {
        int* tmp = (int*)malloc(sizeof(int));
        if (!tmp)
            goto free;
        *tmp = inp_int_s(NULL, 0, 500);
        if (!list_put(szenny, tmp)) {
            free(tmp);
            goto free;
        }
    }

    for (int i = 0; i<n; i++) {
        int* tmp = (int*)list_get(szenny, i);
        if (!tmp)
            goto free2;
        if (*tmp > 100 && !riad) {
            riad = true;
            int* tmp2 = (int*)malloc(sizeof(int));
            if (!tmp2)
                goto free2;
            *tmp2 = i + 1;
            if (!list_put(riado, tmp2)) {
                free(tmp2);
                goto free2;
            }
        } else if (*tmp <= 100)
            riad = false;
    }

    printf("%d ", list_len(riado));
    for (int i = 0; i<list_len(riado); i++) {
        printf("%d ", *(int*)list_get(riado, i));
    }


free2:
    list_free(riado);

free:
    list_free(szenny);

    return 0;
}


