// self
#include "list.h"
// implementation
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h> // SIZE_MAX
#include "small_lib.h" // print

List *list_make(void) {
    return list_make_sz(DEF_SIZE);
}

List *list_make_sz(size_t size) {
    List *list = (List*)malloc(sizeof(List));
    if (!list_init_sz(list, size)) {
        free(list);
        return NULL;
    }
    return list;
}

bool list_init(List *list) {
    return list_init_sz(list, DEF_SIZE);
}

bool list_init_sz(List *list, size_t size) {
    if (!list) {
        err("NPE");
        return false;
    }
    if (size == 0) {
        list->arr = NULL;
        list->len = 0;
        list->sz = 0;
        return true;
    }
    /* if (list->sz || list->len || list->arr) {
        err("List is not in a null state");
        return false;
    } */
    void **arr = (void**)malloc(sizeof(*arr) * size);
    if (!arr) {
        err("malloc failed");
        return false;
    }

    list->arr = arr;
    list->len = 0;
    list->sz = size;
    return true;
}

void list_free(List *list) {
    list_free_fn(list, free);
}
void list_free_fn(List *list, void (*free_fn)(void*)) {
    list_nuke_fn(list, free_fn);
    free(list);
}
void list_nuke(List *list) {
    list_nuke_fn(list, free);
}
void list_nuke_fn(List *list, void (*free_fn)(void*)) {
    if (!list) return;
    
    void **arr = list->arr;
    size_t len = list->len;
    list->sz = 0;
    list->len = 0;
    if (!arr)
        return;
    
    if (free_fn)
        for (size_t i = 0; i < len; i++) {
            free_fn(arr[i]);
        }
    else
        err("items leaked");
    free(arr);
    list->arr = NULL;
}

void **list_free_get(List *list) {
    if (!list) {
        return NULL;
    }
    void** arr = list->arr;
    free(list);
    return arr;
}

void *list_get(const List *list, size_t idx) {
    if (!list) { err("NPE"); /* null pointer */ return NULL; }
    if (idx >= list->len) { err("IOB"); /* index out of bounds */ return NULL; }
    return list->arr[idx];
}

void **list_get_all(List *list) {
    if (!list) {
        err("NPE");
        return NULL;
    }
    return list->arr;
}

size_t list_len(const List *list) {
    return list ? list->len : 0;
}

bool list_put(List *list, void *val) {
    if (!list || !val) {
        err("NPE");
        return false;
    }

    if (list->sz == list->len) {
        if (!list_grow(list)) {
            err("reserve failed");
            return false;
        }
    }

    list->arr[list->len] = val;
    list->len++;
    return true;
}

bool list_ins(List *list, void *item, size_t idx) {
    if (!list || !item) {
        err("NPE");
        return false;
    }
    if (idx > list->len) {
        err("IOB");
        return false;
    }

    if (list->len == list->sz) {
        if (!list_grow(list)) {
            err("reserve failed");
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

void *list_pop(List *list) {
    if (!list) {
        err("NPE");
        return NULL;
    }

    size_t len = list->len;
    if (len == 0) {
        err("IOB");
        return NULL;
    }

    void **arr = list->arr;
    void *item = arr[len - 1];
    if (!item) {
        err("NPE");
        return NULL;
    }

    arr[len - 1] = NULL;
    list->len = len - 1;
    return item;
}

void *list_rem(List *list, size_t idx) {
    if (!list) {
        err("NPE");
        return NULL;
    }

    size_t len = list->len;
    if (idx >= len) {
        err("IOB");
        return NULL;
    }

    void **arr = list->arr;
    void *item = arr[idx];

    for (size_t i = idx; i < len - 1; i++) {
        arr[i] = arr[i + 1];
    }

    list->len = len - 1;
    arr[len - 1] = NULL;
    return item;
}

void *list_rep(List *list, void *val, size_t idx) {
    void *ret = list_get(list, idx);
    // could only be Null on NPE or IOB, what would be checked here too
    if (!ret || !val)
        return NULL;
    list->arr[idx] = val;
    return ret;
}
// reserve - basic
bool list_reserve(List *list, size_t size) {
    if (!list) {
        err("NPE");
        return false;
    }
    if (list->sz >= size)
        return true;

    void **arr = (void**)realloc(list->arr, (sizeof *arr) * size);
    if (!arr)
        return false;
    list->arr = arr;
    list->sz = size;
    return true;
}
bool list_grow(List *list) {
    if (list->sz > SIZE_MAX / 2) return false;
    return list_reserve(list, list->sz == 0 ? DEF_SIZE : list->sz * 2);
}
// growing
bool list_resize(List *list, size_t size, size_t byte) {
    if (!list) {
        err("NPE");
        return false;
    }
    if (byte == 0) {
        err("to reserve memory we needs size");
        return false;
    }
    if (list->len == size) {
        return true;
    }
    if (list->len > size) {
        err("Use truncate/clear for batch deleting items");
        return false;
    }

    bool res = list_reserve(list, size);
    if (!res)
        return false;
    void **arr = list->arr;
    size_t len = list->len, i;
    for (i = len; i < size; i++)
        // here we assume caller Knows that we allocated this
        if (!(arr[i] = (void*)malloc(byte)))
            goto corre;

    list->len = size;
    return true;

corre:
    err("malloc fail");
    while (i-- > len) free(arr[i]);
    return false;
}

bool list_resize_auto(List *list) {
    if (!list) {
        err("NPE");
        return false;
    }

    size_t sz = list->sz, len = list->len;

    while (sz / 2 > len) {
        sz /= 2;
    }
    return list_truncate(list, sz);
}
// clearing
bool list_clear(List *list) {
    return list_clear_fn(list, free);
}
bool list_clear_fn(List *list, void (*free_fn)(void*)) {
    if (!list || !free_fn) {
        err("NPE");
        return false;
    }
    list_nuke_fn(list, free_fn);
    return true;
}
// for shrinking
bool list_truncate(List *list, size_t size) {
    return list_truncate_fn(list, size, free);
}
bool list_truncate_fn(List *list, size_t size, void (*free_fn)(void*)) {
    if (!list || !free_fn) {
        err("NPE");
        return false;
    }
    size_t old = list->sz;
    if (old == size)
        return true;
    else if (old < size)
        return false;
    if (size == 0) {
        err("use clear instead of truncate to 0");
        return false;
    }

    void **data = list->arr;
    // free remaining items if shrinking below current length
    if (list->len > size) {
        for (size_t i = size; i < list->len; i++) {
            free_fn(data[i]);
        }
        list->len = size;
    }

    void **arr = (void**)realloc(data, (sizeof *arr) * size);
    if (!arr) {
        err("realloc failed");
        return false; // shuld return partial true (since we did free stuff)
    }
    list->arr = arr;
    list->sz = size;
    return true;

}

void *list_head(const List *list) {
    return list_get(list, 0);
}

void *list_tail(const List *list) {
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

void list_print(const List *list) {
    if (!list) {
        err("NPE");
        return;
    }

    printf("list with length: %zu size: %zu\n", list->len, list->sz);

    for (size_t i = 0; i < list->len; i++) {
        int *val = (int*)list->arr[i];
        if (val)
            printf("list[%zu] : %d\n", i, *val);
        else
            err("NPE");
    }
}



const ListVTable LIST_VTABLE = {
    .make = list_make,
    .make_sz = list_make_sz,
    .init = list_init,
    .init_sz = list_init_sz,
    
    .reserve = list_reserve,
    .grow = list_grow,
    .resize = list_resize,
    .resize_auto = list_resize_auto,
    .truncate = list_truncate,
    .truncate_fn = list_truncate_fn,
    .clear_fn = list_clear_fn,
    .clear = list_clear,
    
    .get = list_get,
    .get_all = list_get_all,
    .len = list_len,
    .put = list_put,
    .ins = list_ins,
    .pop = list_pop,
    .rem = list_rem,
    .rep = list_rep,
    .head = list_head,
    .tail = list_tail,
    
    .free = list_free,
    .free_fn = list_free_fn,
    .nuke = list_nuke,
    .nuke_fn = list_nuke_fn,
    .free_get = list_free_get,
    
    .print = list_print
};


