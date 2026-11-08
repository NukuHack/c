// self
#include "list.h"
// implementation
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "small_lib.h"
#include "string.h"

struct List {
    size_t len;
    size_t sz;
    void** arr;
};

int list_main(void) {
    List* list = LIST_VTABLE.make();
    if (!list) {
        err("list creation failed");
        return 1;
    }

    int inp = inp_int_s("how much input: ", 3, 100);

    for (int i = 0; i < inp; i++) {
        int tmp = inp_int();
        int* val = (int*)malloc(sizeof(tmp));
        if (!val) {
            err("malloc failed on item");
            goto free;
        }
        *val = tmp;
        if (!LIST_VTABLE.put(list, val)) {
            free(val);
            err("failed put in list");
            goto free;
        }
    }
    LIST_VTABLE.print(list);

free:
    LIST_VTABLE.free(list);
    list = NULL;

    return 0;
}

List* list_make(void) {
    return list_make_sz(DEF_SIZE);
}

List* list_make_sz(size_t size) {
    if (size < 1) { // check
        err("too small");
        return NULL;
    }

    List* list = (List*)malloc(sizeof(List));
    if (!list) {
        err("malloc failed");
        return NULL;
    }

    void **arr = (void**)malloc(sizeof(*arr) * size);
    if (!arr) {
        err("malloc failed");
        free(list);
        return NULL;
    }

    list->arr = arr;
    list->len = 0;
    list->sz = size;
    return list;
}

void list_free(List* list) {
    list_free_fn(list, free);
}

void list_free_fn(List* list, void (*free_fn)(void*)) {
    if (list) {
        void** arr = list->arr;
        if (arr) {
            if (free_fn) {
                for (size_t i = 0; i < list->len; i++) {
                    free_fn(arr[i]);
                }
            }
            free(arr);
        }
        free(list);
    }
}

void** list_free_get(List* list) {
    if (!list) {
        return NULL;
    }
    void** arr = list->arr;
    free(list);
    return arr;
}

void* list_get(const List* list, size_t idx) {
    if (!list) { err("NPE"); /* null pointer */ return NULL; }
    if (idx >= list->len) { err("IOB"); /* index out of bounds */ return NULL; }
    return list->arr[idx];
}

void** list_get_all(List* list) {
    if (!list) {
        err("NPE");
        return NULL;
    }
    return list->arr;
}

size_t list_len(const List* list) {
    return list ? list->len : 0;
}

bool list_put(List* list, void* val) {
    if (!list || !val) {
        err("NPE");
        return false;
    }

    if (list->sz == list->len) {
        if (!list_resize(list, list->sz * 2)) {
            err("resize failed");
            return false;
        }
    }

    list->arr[list->len] = val;
    list->len++;
    return true;
}

bool list_ins(List* list, void* item, size_t idx) {
    if (!list) {
        err("NPE");
        return false;
    }
    if (!item) {
        err("cannot insert NULL");
        return false;
    }
    if (idx > list->len) {
        err("IOB");
        return false;
    }

    if (list->len == list->sz) {
        if (!list_resize(list, list->sz * 2)) {
            err("resize failed");
            return false;
        }
    }

    // shift elements right
    for (size_t i = list->len; i > idx; i--) {
        list->arr[i] = list->arr[i - 1];
    }

    list->arr[idx] = item;
    list->len++;
    return true;
}

void* list_pop(List* list) {
    if (!list) {
        err("NPE");
        return NULL;
    }

    size_t len = list->len;
    if (len == 0) {
        err("IOB");
        return NULL;
    }

    void** arr = list->arr;
    void* item = arr[len - 1];
    if (!item) {
        err("NPE");
        return NULL;
    }

    arr[len - 1] = NULL;
    list->len = len - 1;
    return item;
}

void* list_rem(List* list, size_t idx) {
    if (!list) {
        err("NPE");
        return NULL;
    }

    size_t len = list->len;
    if (idx >= len) {
        err("IOB");
        return NULL;
    }

    void** arr = list->arr;
    void* item = arr[idx];

    for (size_t i = idx; i < len - 1; i++) {
        arr[i] = arr[i + 1];
    }

    list->len = len - 1;
    arr[len - 1] = NULL;
    return item;
}

void* list_rep(List* list, void* val, size_t idx) {
    void* ret = list_get(list, idx);
    if (!ret) {
        // could only be Null on NPE or IOB, what would be checked here too
        return NULL;
    }
    list->arr[idx] = val;
    return ret;
}

bool list_resize(List* list, size_t size) {
    if (!list) {
        err("NPE");
        return false;
    }
    if (size < 1) {
        err("too smol");
        return false;
    }

    size_t old = list->sz;
    if (old == size)
        return true;
    else if (old > size)
        return false;

    void** arr = (void**)realloc(list->arr, (sizeof *arr) * size);
    if (!arr) {
        err("realloc fail");
        return false;
    }
    list->arr = arr;
    list->sz = size;
    return true;
}

bool list_turncate(List* list, size_t size) {
    return list_turncate_fn(list, size, free);
}
bool list_turncate_fn(List* list, size_t size, void (*free_fn)(void*)) {
    if (!list) {
        err("NPE");
        return false;
    }
    // you may even resize it to 0 - freeing the entire thing

    size_t old = list->sz;
    if (old == size)
        return true;
    else if (old < size)
        return false;

    void** data = list->arr;
    // free remaining items if shrinking below current length
    if (list->len > size) {
        for (size_t i = size; i < list->len; i++) {
            free_fn(data[i]);
        }
        list->len = size;
    }

    void** arr = (void**)realloc(data, (sizeof *arr) * size);
    if (!arr) {
        err("realloc failed");
        return false; // shuld return partial true (since we did free stuff)
    }
    list->arr = arr;
    list->sz = size;
    return true;

}

bool list_resize_a(List* list) {
    if (!list) {
        err("NPE");
        return false;
    }

    size_t sz = list->sz;
    size_t n_sz = sz;
    size_t len = list->len;

    while (n_sz / 2 > len) {
        n_sz /= 2;
    }

    if (sz == n_sz)
        return true;

    void** arr = (void**)realloc(list->arr, (sizeof *arr) * n_sz);
    if (!arr) {
        err("realloc failed");
        return false;
    }

    list->sz = n_sz;
    list->arr = arr;
    return true;
}

void* list_head(const List* list) {
    return list_get(list, 0);
}

void* list_tail(const List* list) {
    if (!list) {
        err("NPE");
        return NULL;
    }
    if (list->len == 0) {
        err("IOB");
        return NULL;
    }
    return list->arr[list->len - 1];
}

void list_print(const List* list) {
    if (!list) {
        err("NPE");
        return;
    }

    printf("list with length: %zu size: %zu\n", list->len, list->sz);

    for (size_t i = 0; i < list->len; i++) {
        int* val = (int*)list->arr[i];
        if (val)
            printf("list[%zu] : %d\n", i, *val);
        else
            err("NPE");
    }
}



