
#ifndef LIST_H
#define LIST_H

#include <stdbool.h> // bool
#include <stddef.h> // size_t

#define DEF_SIZE 32

typedef struct List_s {
    size_t len;
    size_t sz;
    void **arr;
} List;

// make list with predefined size
List* list_make(void);
// make list with input size
List* list_make_sz(size_t size);
// init a list with default size
bool list_init(List *list);
// init a list with custom size - using it as an array LoL
bool list_init_sz(List *list, size_t size);

// reserves place, only storage might grow
bool list_reserve(List *list, size_t size);
// small wrapper for reserve (0 handling)
bool list_grow(List *list);
// resize to new length, return is : is_success, DOES MALLOC
bool list_resize(List *list, size_t size, size_t byte);
// resize to auto size, return is : is_success, only shrinks
bool list_resize_auto(List *list);
// when you want to resize to smaller size
bool list_truncate(List *list, size_t size);
// when you want to do the resize, and free at once
bool list_truncate_fn(List *list, size_t size, void (*free_fn)(void*));
// clear the entire thing, freeing it, it even free-s the storage
bool list_clear_fn(List *list, void (*free_fn)(void*));
// same but only for primitives
bool list_clear(List *list);

// get a value of the list, so NULL = fail, or the value Was NULL
void *list_get(const List *list, size_t idx);
// get all in an array !!! DO NOT FREE, list still owns it, but you may modify it
// - free it if you want just make sure to kill the List object beforehand
void **list_get_all(List *list);
// returns the length 0 for NULL
size_t list_len(const List *list);
// put a value to list, false = fail
bool list_put(List *list, void *val);
// push to a specified index - insert
bool list_ins(List *list, void *val, size_t idx);
// pop the tail off the list
void *list_pop(List *list);
// remove from index
void *list_rem(List *list, size_t idx);
// replace the idx-d value with the new one, NULL = fail, or could not get value from that idx
void *list_rep(List *list, void *val, size_t idx);
// get 1st (idx 0) item of the list
void *list_head(const List *list);
// get last (idx len) item of the list
void *list_tail(const List *list);

// free memory
// frees thrugh the pointers, do not use with NON primitives
void list_free(List *list);
// free memory, can be used for custom inner data
void list_free_fn(List *list, void (*free_fn)(void*));
// free internal array, nothing else
void list_nuke(List *list);
// ^^ with function
void list_nuke_fn(List *list, void (*free_fn)(void*));
// free List memory, return the pointer to inner data for caller to free
void **list_free_get(List *list);
// ^ this does not have "stack" version, since that is just the "get_all"

#endif /* LIST_H */

