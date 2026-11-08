
#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include <stddef.h>

#define DEF_SIZE 10

// size of this is 2*int + pointer
typedef struct List List;
typedef void (*free_fn)(void*);

// make list with predefined size
List* list_make(void);
// make list with input size
List* list_make_sz(size_t size);
// resize to new length, return is : is_success
bool list_resize(List* list, size_t size);
// resize to auto size, return is : is_success
bool list_resize_a(List* list);
// when you want to resize to smaller size
bool list_turncate(List* list, size_t size);
// when you want to do the resize, and free at once
bool list_turncate_fn(List* list, size_t size, free_fn);
// get a value of the list, so NULL = fail, or the value Was NULL
void* list_get(const List* list, size_t idx);
// get all in an array
// DO NOT FREE, list still owns it, but you may modify it
void** list_get_all(List* list);
// returns the length 0 for NULL
size_t list_len(const List* list);
// put a value to list, false = fail
bool list_put(List* list, void* val);
// push to a specified index - insert
bool list_ins(List* list, void* val, size_t idx);
// pop the tail off the list
void* list_pop(List* list);
// remove from index
void* list_rem(List* list, size_t idx);
// replace the idx-d value with the new one, NULL = fail, or could not get value from that idx
void* list_rep(List* list, void* val, size_t idx);
// get 1st (idx 0) item of the list
void* list_head(const List* list);
// get last (idx len) item of the list
void* list_tail(const List* list);
// free memory
// frees thrugh the pointers, do not use with NON primitives
void list_free(List* list);
// free memory, can be used for custom inner data
void list_free_fn(List* list, free_fn);
// free List memory, return the pointer to inner data for caller to free
void** list_free_get(List* list);
// print entire "array" - casts the inner data to "int" without care
// do not use for anything that is NOT a primitive
void list_print(const List* list);

typedef struct {
    List* (*make)(void);
    List* (*make_sz)(size_t size);
    bool (*resize)(List* list, size_t size);
    bool (*resize_a)(List* list);
    void* (*get)(const List* list, size_t idx);
    void** (*get_all)(List* list);
    size_t (*len)(const List* list);
    bool (*put)(List* list, void* val);
    void* (*pop)(List* list);
    void* (*rem)(List* list, size_t idx);
    void* (*rep)(List* list, void* val, size_t idx);
    void* (*head)(const List* list);
    void* (*tail)(const List* list);
    void (*free)(List* list);
    void (*free_fn)(List* list, free_fn);
    void** (*free_get)(List* list);
    void (*print)(const List* list);
} ListVTable;

static const ListVTable LIST_VTABLE = {
    .make = list_make,
    .make_sz = list_make_sz,
    .resize = list_resize,
    .resize_a = list_resize_a,
    .get = list_get,
    .get_all = list_get_all,
    .len = list_len,
    .put = list_put,
    .pop = list_pop,
    .rem = list_rem,
    .rep = list_rep,
    .head = list_head,
    .tail = list_tail,
    .free = list_free,
    .free_fn = list_free_fn,
    .free_get = list_free_get,
    .print = list_print
};


#endif /* LIST_H */

