// — void*-backed dynamic array macros.
//
// Works with any struct shaped like:
//     typedef struct { T *items; size_t count; size_t capacity; } Foo;
//
// Difference from da.h: the heavy logic (reserve/realloc/growth,
// insert/remove, resize_byte rollback, etc.) lives in `static` functions
// operating on void*, so it is emitted once per TU instead of once per
// call site. Call-site type safety is preserved because every macro
// extracts `sizeof *(xs)->items` at the call site and, under GNU/Clang,
// binds the appended value to `__typeof__(*(xs)->items)`.

#ifndef DA_H_
#define DA_H_

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifndef DA_INIT_CAP
#define DA_INIT_CAP 256
#endif

#ifndef DA_OOM
#define DA_OOM() abort()
#endif

// ---------------------------------------------------------------------------
// Core implementations — real functions, one emitted copy each.
// ---------------------------------------------------------------------------

static void *da__reserve_impl(void *items, size_t *cap, size_t elem_size, size_t needed)
{
    if (*cap >= needed) return items;
    size_t new_cap = *cap ? *cap : DA_INIT_CAP;
    while (new_cap < needed) new_cap *= 2;
    void *p = realloc(items, new_cap * elem_size);
    if (!p) DA_OOM();
    *cap = new_cap;
    return p;
}

static void *da__append_impl(void *items, size_t *count, size_t *cap, size_t elem_size, const void *elem)
{
    items = da__reserve_impl(items, cap, elem_size, *count + 1);
    memcpy((char *)items + (*count) * elem_size, elem, elem_size);
    (*count)++;
    return items;
}

static void *da__append_many_impl(void *items, size_t *count, size_t *cap, size_t elem_size, const void *src, size_t n)
{
    items = da__reserve_impl(items, cap, elem_size, *count + n);
    memcpy((char *)items + (*count) * elem_size, src, n * elem_size);
    *count += n;
    return items;
}

static void *da__insert_impl(void *items, size_t *count, size_t *cap, size_t elem_size, size_t i, const void *elem)
{
    assert(i <= *count);
    items = da__reserve_impl(items, cap, elem_size, *count + 1);
    char *base = (char *)items;
    memmove(base + (i + 1) * elem_size, base + i * elem_size, (*count - i) * elem_size);
    memcpy(base + i * elem_size, elem, elem_size);
    (*count)++;
    return items;
}

static void da__remove_impl(void *items, size_t *count, size_t elem_size, size_t i)
{
    assert(i < *count);
    char *base = (char *)items;
    memmove(base + i * elem_size, base + (i + 1) * elem_size, (*count - i - 1) * elem_size);
    (*count)--;
}

static void da__remove_unordered_impl(void *items, size_t *count, size_t elem_size, size_t i)
{
    assert(i < *count);
    char *base = (char *)items;
    if (i != *count - 1)
        memcpy(base + i * elem_size, base + (*count - 1) * elem_size, elem_size);
    (*count)--;
}

static void *da__resize_impl(void *items, size_t *count, size_t *cap, size_t elem_size, size_t n)
{
    items = da__reserve_impl(items, cap, elem_size, n);
    if (n > *count)
        memset((char *)items + (*count) * elem_size, 0, (n - *count) * elem_size);
    *count = n;
    return items;
}

// For containers of pointers: malloc(byte) into each new slot, rolling back
// already-malloc'd slots if one fails, then DA_OOM().
static void *da__resize_byte_impl(void *items, size_t *count, size_t *cap, size_t elem_size, size_t n, size_t byte)
{
    if (n <= *count) { *count = n; return items; }
    items = da__reserve_impl(items, cap, elem_size, n);
    size_t old = *count, i = old;
    char *base = (char *)items;
    while (i < n) {
        void *p = malloc(byte);
        if (!p) {
            for (size_t j = old; j < i; j++)
                free(*(void **)(base + j * elem_size));
            DA_OOM();
        }
        *(void **)(base + i * elem_size) = p;
        i++;
    }
    *count = n;
    return items;
}

static void *da__resize_auto_impl(void *items, size_t count, size_t *cap, size_t elem_size)
{
    size_t nc = *cap;
    while (nc / 2 > count) nc /= 2;
    if (nc < *cap) {
        if (nc == 0) { free(items); *cap = 0; return NULL; }
        void *p = realloc(items, nc * elem_size);
        if (!p) DA_OOM();
        *cap = nc;
        return p;
    }
    return items;
}

static void *da__truncate_impl(void *items, size_t *count, size_t *cap, size_t elem_size, size_t n)
{
    if (n < *count) *count = n;
    if (n == 0) { free(items); *cap = 0; return NULL; }
    if (n < *cap) {
        void *p = realloc(items, n * elem_size);
        if (!p) DA_OOM();
        *cap = n;
        return p;
    }
    return items;
}

// ---------------------------------------------------------------------------
// Macros
// ---------------------------------------------------------------------------

#define da_reserve(xs, n)                                                   \
    ((xs)->items = da__reserve_impl((xs)->items, &(xs)->capacity, sizeof *(xs)->items, (n)))

#if defined(__GNUC__) || defined(__clang__)
// GNU path: temp of the element type enforces compile-time type checking
// and allows rvalues (e.g. da_append(&xs, 42)).
#define da_append(xs, x)                                                    \
    do {                                                                    \
        __typeof__(*(xs)->items) da__t = (x);                               \
        (xs)->items = da__append_impl((xs)->items, &(xs)->count, &(xs)->capacity, sizeof *(xs)->items, &da__t);         \
    } while (0)
#define da_insert(xs, i, x)                                                 \
    do {                                                                    \
        __typeof__(*(xs)->items) da__t = (x);                               \
        (xs)->items = da__insert_impl((xs)->items, &(xs)->count, &(xs)->capacity, sizeof *(xs)->items, (i), &da__t);    \
    } while (0)
#define da_null_of(xs) ((__typeof__((xs)->items))NULL)
#else
// Portable path: requires x to be an lvalue (addressable).
#define da_append(xs, x)                                                    \
    ((xs)->items = da__append_impl((xs)->items, &(xs)->count, &(xs)->capacity, sizeof *(xs)->items, &(x)))
#define da_insert(xs, i, x)                                                 \
    ((xs)->items = da__insert_impl((xs)->items, &(xs)->count, &(xs)->capacity, sizeof *(xs)->items, (i), &(x)))
#define da_null_of(xs) NULL
#endif

#define da_append_many(xs, src, n)                                          \
    ((xs)->items = da__append_many_impl((xs)->items, &(xs)->count, &(xs)->capacity, sizeof *(xs)->items, (src), (n)))

#define da_remove(xs, i)                                                    \
    da__remove_impl((xs)->items, &(xs)->count, sizeof *(xs)->items, (i))

#define da_remove_unordered(xs, i)                                          \
    da__remove_unordered_impl((xs)->items, &(xs)->count, sizeof *(xs)->items, (i))

#define da_resize(xs, n)                                                    \
    ((xs)->items = da__resize_impl((xs)->items, &(xs)->count, &(xs)->capacity, sizeof *(xs)->items, (n)))

#define da_clear(xs) ((xs)->count = 0)

#define da_free(xs)                                                         \
    do {                                                                    \
        free((xs)->items);                                                  \
        (xs)->items = NULL;                                                 \
        (xs)->count = 0;                                                    \
        (xs)->capacity = 0;                                                 \
    } while (0)

// Return T* (not void*) under GNU/Clang, so *da_get(&xs, 0) = v works.
#define da_first(xs)                                                        \
    ((xs)->count ? &(xs)->items[0] : da_null_of(xs))
#define da_last(xs)                                                         \
    ((xs)->count ? &(xs)->items[(xs)->count - 1] : da_null_of(xs))
#define da_get(xs, i)                                                       \
    ((size_t)(i) < (xs)->count ? &(xs)->items[(i)] : da_null_of(xs))
#define da_pop(xs)                                                          \
    ((xs)->count ? &(xs)->items[--(xs)->count] : da_null_of(xs))

#define da_foreach(Type, it, xs)                                            \
    for (Type *it = (xs)->items; it && it < (xs)->items + (xs)->count; ++it)

#define da_sort(xs, cmp)                                                    \
    qsort((xs)->items, (xs)->count, sizeof *(xs)->items, (cmp))

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

#define da_init(xs)                                                         \
    do {                                                                    \
        (xs)->items = NULL;                                                 \
        (xs)->count = 0;                                                    \
        (xs)->capacity = 0;                                                 \
    } while (0)

#define da_init_sz(xs, n) do { da_init(xs); da_reserve((xs), (n)); } while (0)

#define da_make_sz_into(out, Type, n) do {                                  \
    (out) = (Type *)malloc(sizeof(Type));                                   \
    if (out) {                                                              \
        (out)->items = NULL;                                                \
        (out)->count = 0;                                                   \
        (out)->capacity = 0;                                                \
        da_reserve((out), (n));                                             \
    }                                                                       \
} while (0)

#define da_make_into(out, Type) da_make_sz_into(out, Type, DA_INIT_CAP)

#if defined(__GNUC__) || defined(__clang__)
#define da_make_sz(Type, n) __extension__ ({                                \
    Type *da__mk_p = (Type *)malloc(sizeof(Type));                          \
    if (da__mk_p) {                                                         \
        da__mk_p->items = NULL;                                             \
        da__mk_p->count = 0;                                                \
        da__mk_p->capacity = 0;                                             \
        da_reserve(da__mk_p, (n));                                          \
    }                                                                       \
    da__mk_p;                                                               \
})
#define da_make(Type) da_make_sz(Type, DA_INIT_CAP)
#define da_free_get(xs) __extension__ ({                                    \
    void *da__fg_items = (xs) ? (xs)->items : NULL;                         \
    free(xs);                                                               \
    da__fg_items;                                                           \
})
#else
#define da_free_get(xs, out)                                                \
    do {                                                                    \
        (out) = (xs) ? (xs)->items : NULL;                                  \
        free(xs);                                                           \
    } while (0)
#endif

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------

#define da_len(xs)     ((xs)->count)
#define da_get_all(xs) ((xs)->items)

#define da_grow(xs)                                                         \
    da_reserve((xs), (xs)->capacity ? (xs)->capacity * 2 : DA_INIT_CAP)

#define da_rep(xs, i, x)                                                    \
    do {                                                                    \
        assert((size_t)(i) < (xs)->count);                                  \
        (xs)->items[(i)] = (x);                                             \
    } while (0)

// ---------------------------------------------------------------------------
// Bulk operations
// ---------------------------------------------------------------------------

#define da_resize_byte(xs, n, byte)                                         \
    ((xs)->items = da__resize_byte_impl((xs)->items, &(xs)->count, &(xs)->capacity, sizeof *(xs)->items, (n), (byte)))

#define da_resize_auto(xs)                                                  \
    ((xs)->items = da__resize_auto_impl((xs)->items, (xs)->count, &(xs)->capacity, sizeof *(xs)->items))

#define da_truncate(xs, n)                                                  \
    ((xs)->items = da__truncate_impl((xs)->items, &(xs)->count, &(xs)->capacity, sizeof *(xs)->items, (n)))

#define da_truncate_fn(xs, n, free_fn)                                      \
    do {                                                                    \
        size_t da__tf_n = (n);                                              \
        size_t da__tf_old = (xs)->count;                                    \
        if (da__tf_n < da__tf_old) {                                        \
            for (size_t da__tf_i = da__tf_n;                                \
                 da__tf_i < da__tf_old; da__tf_i++)                         \
                (free_fn)((xs)->items[da__tf_i]);                           \
            (xs)->count = da__tf_n;                                         \
        }                                                                   \
        (xs)->items = da__truncate_impl((xs)->items, &(xs)->count,          \
                       &(xs)->capacity, sizeof *(xs)->items, da__tf_n);     \
    } while (0)

#define da_clear_fn(xs, free_fn)                                            \
    do {                                                                    \
        for (size_t da__cf_i = 0; da__cf_i < (xs)->count; da__cf_i++)       \
            (free_fn)((xs)->items[da__cf_i]);                               \
        (xs)->count = 0;                                                    \
    } while (0)

#define da_free_fn(xs, free_fn)                                             \
    do {                                                                    \
        for (size_t da__ff_i = 0; da__ff_i < (xs)->count; da__ff_i++)       \
            (free_fn)((xs)->items[da__ff_i]);                               \
        da_free(xs);                                                        \
    } while (0)

// ---------------------------------------------------------------------------
// Print
// ---------------------------------------------------------------------------

#ifndef DA_PRINT
#  include <stdio.h>
#  define DA_PRINT(...) printf(__VA_ARGS__)
#endif

#define da_print(xs, fmt)                                                   \
    do {                                                                    \
        DA_PRINT("len: %zu cap: %zu\n",                                     \
                 (size_t)(xs)->count, (size_t)(xs)->capacity);              \
        for (size_t da__pr_i = 0; da__pr_i < (xs)->count; da__pr_i++)       \
            DA_PRINT("[" fmt "]\n", (xs)->items[da__pr_i]);                 \
    } while (0)

#endif // DA_H_
