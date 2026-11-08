// — generic dynamic array macros.
//
// Works with any struct shaped like:
//     typedef struct { T *items; size_t count; size_t capacity; } Foo;
//
// All macros take a pointer to the container (e.g. &xs). Every local inside
// a macro is named da__<macro>_<role> so nested expansions cannot shadow
// each other's arguments.

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
// API reference
// ---------------------------------------------------------------------------
// Every entry below is a preprocessor macro. `xs` is a pointer to the
// container, never the container itself. `T` is the element type.
// `T*` pointer to the items array and stay valid until the
// next append / reserve / clear on that container.
//
// CORE
//   da_reserve(xs, needed)              -> void      ensure capacity for `needed` total
//   da_append(xs, x)                    -> void      append one element
//   da_append_many(xs, src, n)          -> void      append n elements from src
//   da_insert(xs, i, x)                 -> void      insert at i, shift tail right (O(n))
//   da_remove(xs, i)                    -> void      remove i, keep order (O(n))
//   da_remove_unordered(xs, i)          -> void      remove i, swap last in (O(1))
//   da_resize(xs, n)                    -> void      set length to n, zero new slots
//   da_clear(xs)                        -> void      length = 0, keep capacity
//   da_free(xs)                         -> void      free backing, reset all fields
//   da_first(xs)                        -> T*|NULL   first element
//   da_last(xs)                         -> T*|NULL   last element
//   da_foreach(Type, it, xs)            -> loop      for (Type *it = ...; ...; ++it)
//   da_sort(xs, cmp)                    -> void      qsort(items, cmp)
//
// LIFECYCLE
//   da_init(xs)                         -> void      zero fields, defer allocation
//   da_init_sz(xs, n)                   -> void      zero fields, reserve n
//   da_make_into(out, Type)             -> void      out = heap container, DEFAULT cap
//   da_make_sz_into(out, Type, n)       -> void      out = heap container, cap n
//   da_make(Type)                       -> Type*     new heap container          [GNU/Clang]
//   da_make_sz(Type, n)                 -> Type*     new heap container, cap n   [GNU/Clang]
//   da_free_get(xs)                     -> void*     free struct, return items   [GNU/Clang]
//   da_free_get(xs, out)                -> void      free struct, out = items  [portable]
//
// ACCESS
//   da_len(xs)                          -> size_t    element count
//   da_get_all(xs)                      -> T*        items array
//   da_get(xs, i)                       -> T*|NULL   element at i
//   da_grow(xs)                         -> void      force one doubling
//   da_pop(xs)                          -> T*|NULL   popped slot (copy out to keep)
//   da_rep(xs, i, x)                    -> void      replace element at i
//
// BULK / ELEMENT-WISE
//   da_resize_byte(xs, n, byte)         -> void      malloc(byte) for each new slot
//   da_resize_auto(xs)                  -> void      shrink capacity toward count
//   da_truncate(xs, n)                  -> void      shrink to n, n=0 frees backing
//   da_truncate_fn(xs, n, free_fn)      -> void      truncate, free_fn on dropped
//   da_clear_fn(xs, free_fn)            -> void      free_fn on all, keep capacity
//   da_free_fn(xs, free_fn)             -> void      free_fn on all, then free
//
// PRINT
//   da_print(xs, fmt)                   -> void      per-element `fmt` (fmt literal)

// ---------------------------------------------------------------------------
// Core
// ---------------------------------------------------------------------------

// Ensures room for at least `needed` elements total (not additional). Growth
// doubles from the current capacity (or DA_INIT_CAP) so da_append stays O(1)
// amortised.
#define da_reserve(xs, needed)                                              \
    do {                                                                    \
        size_t da__res_need = (needed);                                     \
        if ((xs)->capacity < da__res_need) {                                \
            size_t da__res_cap = (xs)->capacity                             \
                               ? (xs)->capacity : DA_INIT_CAP;              \
            while (da__res_cap < da__res_need) da__res_cap *= 2;            \
            void *da__res_p = realloc(                                      \
                (xs)->items, da__res_cap * sizeof *(xs)->items);            \
            if (!da__res_p) DA_OOM();                                       \
            (xs)->items    = da__res_p;                                     \
            (xs)->capacity = da__res_cap;                                   \
        }                                                                   \
    } while (0)

#define da_append(xs, x)                                                    \
    do {                                                                    \
        da_reserve((xs), (xs)->count + 1);                                  \
        (xs)->items[(xs)->count++] = (x);                                   \
    } while (0)

// memcpy (not memmove): `src` must not alias xs->items.
#define da_append_many(xs, src, n)                                          \
    do {                                                                    \
        size_t da__am_n = (n);                                              \
        da_reserve((xs), (xs)->count + da__am_n);                           \
        memcpy((xs)->items + (xs)->count, (src),                            \
               da__am_n * sizeof *(xs)->items);                             \
        (xs)->count += da__am_n;                                            \
    } while (0)

#define da_insert(xs, i, x)                                                 \
    do {                                                                    \
        size_t da__ins_i = (i);                                             \
        assert(da__ins_i <= (xs)->count);                                   \
        da_reserve((xs), (xs)->count + 1);                                  \
        memmove(&(xs)->items[da__ins_i + 1], &(xs)->items[da__ins_i],       \
                ((xs)->count - da__ins_i) * sizeof *(xs)->items);           \
        (xs)->items[da__ins_i] = (x);                                       \
        (xs)->count += 1;                                                   \
    } while (0)

#define da_remove(xs, i)                                                    \
    do {                                                                    \
        size_t da__rem_i = (i);                                             \
        assert(da__rem_i < (xs)->count);                                    \
        memmove(&(xs)->items[da__rem_i], &(xs)->items[da__rem_i + 1],       \
                ((xs)->count - da__rem_i - 1) * sizeof *(xs)->items);       \
        (xs)->count -= 1;                                                   \
    } while (0)

// O(1); moves the last element into the hole. Order is not preserved.
#define da_remove_unordered(xs, i)                                          \
    do {                                                                    \
        size_t da__remu_i = (i);                                            \
        assert(da__remu_i < (xs)->count);                                   \
        (xs)->items[da__remu_i] = (xs)->items[(xs)->count - 1];             \
        (xs)->count -= 1;                                                   \
    } while (0)

// Newly exposed slots are zero-filled so they are safe to read or free.
#define da_resize(xs, n)                                                    \
    do {                                                                    \
        size_t da__rsz_n = (n);                                             \
        da_reserve((xs), da__rsz_n);                                        \
        if (da__rsz_n > (xs)->count)                                        \
            memset(&(xs)->items[(xs)->count], 0,                            \
                   (da__rsz_n - (xs)->count) * sizeof *(xs)->items);        \
        (xs)->count = da__rsz_n;                                            \
    } while (0)

#define da_clear(xs) do { (xs)->count = 0; } while (0)

#define da_free(xs)                                                         \
    do {                                                                    \
        free((xs)->items);                                                  \
        (xs)->items    = NULL;                                              \
        (xs)->count    = 0;                                                 \
        (xs)->capacity = 0;                                                 \
    } while (0)

#define da_first(xs) ((xs)->count ? &(xs)->items[0] : NULL)
#define da_last(xs)  ((xs)->count ? &(xs)->items[(xs)->count - 1] : NULL)

// The `it &&` guard avoids NULL + 0 pointer arithmetic (UB in strict C) on
// a zero-capacity container.
#define da_foreach(Type, it, xs)                                            \
    for (Type *it = (xs)->items; it && it < (xs)->items + (xs)->count; ++it)

#define da_sort(xs, cmp)                                                    \
    qsort((xs)->items, (xs)->count, sizeof *(xs)->items, (cmp))

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

// Allocation is deferred until the first append or reserve.
#define da_init(xs)                                                         \
    do {                                                                    \
        (xs)->items    = NULL;                                              \
        (xs)->count    = 0;                                                 \
        (xs)->capacity = 0;                                                 \
    } while (0)

#define da_init_sz(xs, n) do { da_init(xs); da_reserve((xs), (n)); } while (0)

// Does not use a statement expression, so it is available on all compilers.
#define da_make_sz_into(out, Type, n) do {                                  \
    (out) = (Type *)malloc(sizeof(Type));                                   \
    if (out) {                                                              \
        (out)->items    = NULL;                                             \
        (out)->count    = 0;                                                \
        (out)->capacity = 0;                                                \
        da_reserve((out), (n));                                             \
    }                                                                       \
} while (0)

#define da_make_into(out, Type) da_make_sz_into(out, Type, DA_INIT_CAP)

#if defined(__GNUC__) || defined(__clang__)
// __extension__ silences the GNU statement-expression warning under
// -std=c99 -pedantic.
#define da_make_sz(Type, n) __extension__ ({                                \
    Type *da__mk_p = (Type *)malloc(sizeof(Type));                          \
    if (da__mk_p) {                                                         \
        da__mk_p->items    = NULL;                                          \
        da__mk_p->count    = 0;                                             \
        da__mk_p->capacity = 0;                                             \
        da_reserve(da__mk_p, (n));                                          \
    }                                                                       \
    da__mk_p;                                                               \
})

#define da_make(Type) da_make_sz(Type, DA_INIT_CAP)

// Stashes the items pointer before free() so the caller never observes a
// use-after-free.
#define da_free_get(xs) __extension__ ({                                    \
    void *da__fg_items = (xs) ? (xs)->items : NULL;                         \
    free(xs);                                                               \
    da__fg_items;                                                           \
})
#else
// Portable fallback: caller supplies an out-parameter for the items pointer.
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

// Returns a pointer to the slot, or NULL if out of range.
#define da_get(xs, i)                                                       \
    ((size_t)(i) < (xs)->count ? &(xs)->items[(i)] : NULL)

// Force one doubling step.
#define da_grow(xs)                                                         \
    da_reserve((xs), (xs)->capacity ? (xs)->capacity * 2 : DA_INIT_CAP)

// Returns a pointer to the popped slot, or NULL if empty. The pointer is
// valid until the next mutation; copy out with `T v = *da_pop(&xs);`.
#define da_pop(xs)                                                          \
    ((xs)->count ? &(xs)->items[--(xs)->count] : NULL)

#define da_rep(xs, i, x)                                                    \
    do {                                                                    \
        size_t da__rep_i = (i);                                             \
        assert(da__rep_i < (xs)->count);                                    \
        (xs)->items[da__rep_i] = (x);                                       \
    } while (0)

// ---------------------------------------------------------------------------
// Bulk operations
// ---------------------------------------------------------------------------

// Only meaningful when T is a pointer type; mallocs `byte` bytes for each
// newly exposed slot. On OOM, the freshly allocated slots are freed before
// DA_OOM() so the container is left in its original state.
#define da_resize_byte(xs, n, byte)                                         \
    do {                                                                    \
        size_t da__rb_n = (n);                                              \
        if (da__rb_n <= (xs)->count) {                                      \
            (xs)->count = da__rb_n;                                         \
        } else {                                                            \
            da_reserve((xs), da__rb_n);                                     \
            size_t da__rb_i = (xs)->count;                                  \
            while (da__rb_i < da__rb_n) {                                   \
                void *da__rb_p = malloc(byte);                              \
                if (!da__rb_p) {                                            \
                    while (da__rb_i > (xs)->count)                          \
                        free((xs)->items[--da__rb_i]);                      \
                    DA_OOM();                                               \
                }                                                           \
                (xs)->items[da__rb_i++] = da__rb_p;                         \
            }                                                               \
            (xs)->count = da__rb_n;                                         \
        }                                                                   \
    } while (0)

// Halves capacity while count <= capacity/2, then reallocs to fit. Never
// grows, so it is safe to call after a shrink.
#define da_resize_auto(xs)                                                  \
    do {                                                                    \
        size_t da__ra_nc = (xs)->capacity;                                  \
        while (da__ra_nc / 2 > (xs)->count) da__ra_nc /= 2;                 \
        if (da__ra_nc < (xs)->capacity) {                                   \
            if (da__ra_nc == 0) {                                           \
                free((xs)->items);                                          \
                (xs)->items    = NULL;                                      \
                (xs)->capacity = 0;                                         \
            } else {                                                        \
                void *da__ra_p = realloc((xs)->items, da__ra_nc             \
                                         * sizeof *(xs)->items);            \
                if (!da__ra_p) DA_OOM();                                    \
                (xs)->items    = da__ra_p;                                  \
                (xs)->capacity = da__ra_nc;                                 \
            }                                                               \
        }                                                                   \
    } while (0)

// Shrink to n elements. n == 0 frees the backing store entirely.
#define da_truncate(xs, n)                                                  \
    do {                                                                    \
        size_t da__tc_n = (n);                                              \
        if (da__tc_n < (xs)->count) (xs)->count = da__tc_n;                 \
        if (da__tc_n == 0) {                                                \
            free((xs)->items);                                              \
            (xs)->items    = NULL;                                          \
            (xs)->capacity = 0;                                             \
        } else if (da__tc_n < (xs)->capacity) {                             \
            void *da__tc_p = realloc((xs)->items,                           \
                                     da__tc_n * sizeof *(xs)->items);       \
            if (!da__tc_p) DA_OOM();                                        \
            (xs)->items    = da__tc_p;                                      \
            (xs)->capacity = da__tc_n;                                      \
        }                                                                   \
    } while (0)

// Same as da_truncate, but invokes free_fn on every dropped element.
#define da_truncate_fn(xs, n, free_fn)                                      \
    do {                                                                    \
        size_t da__tcf_n   = (n);                                           \
        size_t da__tcf_old = (xs)->count;                                   \
        if (da__tcf_n < da__tcf_old) {                                      \
            for (size_t da__tcf_i = da__tcf_n;                              \
                 da__tcf_i < da__tcf_old; da__tcf_i++)                      \
                (free_fn)((xs)->items[da__tcf_i]);                          \
            (xs)->count = da__tcf_n;                                        \
        }                                                                   \
        if (da__tcf_n == 0) {                                               \
            free((xs)->items);                                              \
            (xs)->items    = NULL;                                          \
            (xs)->capacity = 0;                                             \
        } else if (da__tcf_n < (xs)->capacity) {                            \
            void *da__tcf_p = realloc((xs)->items,                          \
                                      da__tcf_n * sizeof *(xs)->items);     \
            if (!da__tcf_p) DA_OOM();                                       \
            (xs)->items    = da__tcf_p;                                     \
            (xs)->capacity = da__tcf_n;                                     \
        }                                                                   \
    } while (0)

// Runs free_fn on every element and resets count. Capacity is preserved so
// the array can be reused without reallocating.
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
