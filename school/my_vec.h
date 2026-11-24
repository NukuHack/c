/**
 * GENERIC dynamic resizable array implementation
 * mainly macros ...
 * but since the types are defined where used : no need for getters !!
 * also it's easier to manually manipulate the inner data (idk why would you)
 * but yeah, you can treat it as any kind of array, and just iter through it or sum'
 */
#if ZERO
/* example struct */
struct Vec {
    size_t capacity;
    size_t count;
    int *data;
}
#endif

#if !defined(MY_VEC_H)
#define MY_VEC_H

#include "my_utils.h"

/* this is pasted to ever place where any kind of memory operation fails */
#if !defined(VEC_OOM_HANDLER)
#  define VEC_OOM_HANDLER \
    perror("out of memory"); \
    exit(1);
#endif


/* small so any testing for resize is easy */
#if !defined(INIT_SIZE)
#    define INIT_SIZE 2
#endif

/**
 * grow the data-array to a pecific size (Atleast)
 * if current is bigger then this is a noop
 */
#define vec_alloc(vec, size) \
    do { \
        size_t cap_ = (vec).capacity; \
        size_t desired_ = (size); \
        if(cap_ < desired_) { \
            cap_ = MAX(INIT_SIZE, desired_); \
            void *p = realloc((vec).data, cap_ * sizeof(*(vec).data)); \
            if (p) { \
                (vec).data = p; \
                (vec).capacity = cap_; \
            } else { \
                VEC_OOM_HANDLER \
            } \
        } \
    } while (0)

/**
 * grow the data-array
 * !! Unconditional !! make sure to check if needed Before
 */
#define vec_grow(vec) \
    do { \
        size_t cap_ = (vec).capacity; \
        cap_ = MAX(INIT_SIZE, cap_ << 1); \
        void *p = realloc((vec).data, cap_ * sizeof(*(vec).data)); \
        if (p) { \
            (vec).data = p; \
            (vec).capacity = cap_; \
        } else { \
            VEC_OOM_HANDLER \
        } \
    } while (0)

/**
 * learned from github, git push --force
 */
#define vec_force_push(vec, item)  (vec).data[(vec).count++] = item

/**
 * push a new item to the data-array, grow if needed
 */
#define vec_push(vec, item) \
    do { \
        if((vec).capacity == (vec).count) vec_grow(vec); \
        vec_force_push(vec, item); \
    } while (0)

/**
 * push bach item arrayto the data-array, grow if needed
 */
#define vec_push_arr(vec, arr, len) \
    do { \
        forx(len) { \
            vec_push(vec, arr[i]); \
        } \
    } while (0)

/**
 * print each item in the data-array, using the FMT
 */
#define vec_print_fmt_n(vec, fmt) \
    do { \
        size_t cou_ = (vec).count; \
        for(size_t i = 0; i<cou_; i++) { \
            printf(fmt"\n", (vec).data[i]); \
        } \
    } while (0)

#define vec_print_fmt(vec, fmt) \
    do { \
        size_t cou_ = (vec).count; \
        for(size_t i = 0; i<cou_; i++) { \
            printf(fmt" ", (vec).data[i]); \
        } \
    } while (0)

/**
 * prints using my_macro.h
 */
#define vec_print_n(vec, ty) vec_print_fmt_n(vec, SWITCH_FMT(ty))
#define vec_print(vec, ty) vec_print_fmt(vec, SWITCH_FMT(ty))

/**
 * makes the count 0, meaning "clears" the data-array
 *  - but in fact it does not really clear it
 *  - so make sure the remaining items are not used again
 */
#define vec_clear(vec) (vec).count = 0

/**
 * frees the data-array, and sets capacity & count to 0
 */
#define vec_destroy(vec) \
    do { \
        if ((vec).data) { \
            free((vec).data); \
            (vec).data = NULL; \
        } \
        (vec).capacity = 0; \
        (vec).count = 0; \
    } while (0)

/**
 * pop-s the last element in the data-array
 * return : ptr to item
 * NULL otherwise
 * impotant : for now we don't resize even if we have No items in any big array
 */
#define vec_pop(vec) \
    (vec).count ? (vec).data + (--(vec).count) : NULL

/**
 * get the idx-th item from it's data-array
 * return : ptr to item
 * NULL otherwise
 * could not make it with only evaulating idx once
 */
#define vec_get(vec, idx) \
     (idx) >= 0 && (vec).count > (size_t)(idx)? (vec).data + (idx) : NULL


// TODO :
// resize-auto  and/or   some kind of shrink method
// insert, to a specific index
// remove, from a specific index
// replace, item with something
// contains
// head & tail

#endif /* MY_VEC_H */


