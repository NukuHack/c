
#if !defined(MY_VEC_H)
#define MY_VEC_H

#include <stdlib.h> // mem stuff
#include <string.h> // batch-mem stuff
#include <stddef.h> // size_t                    
#include <stdio.h> // io

#if !defined(VEC_OOM_HANDLER)
#  define VEC_OOM_HANDLER \
    fputs("out of memory\n", stderr); \
    abort();
#endif

#if 0
struct Vec {
    size_t capacity;
    size_t count;
    int *data;
}
#endif

#if !defined(INIT_SIZE)
#    define INIT_SIZE 2
#endif

#define vec_f_push(vec, item) \
    (vec).data[(vec).count++] = item

#define vec_make(vec, i) \
    do { \
        size_t cap_ = (vec).capacity; \
        size_t desired_ = (i); \
        if(cap_ < desired_) { \
            cap_ = desired_ < INIT_SIZE ? INIT_SIZE : desired_; \
            void *p = realloc((vec).data, cap_ * sizeof(*(vec).data)); \
            if (p) { \
                (vec).data = p; \
                (vec).capacity = cap_; \
            } else { \
                VEC_OOM_HANDLER \
            } \
        } \
    } while (0)

#define vec_push(vec, item) \
    do { \
        size_t cap_ = (vec).capacity; \
        size_t cou_ = (vec).count; \
        if(cap_ > cou_) { \
            vec_f_push(vec, item); \
        } else { \
            cap_ = cap_ ? cap_ * 2 : INIT_SIZE; \
            void *p = realloc((vec).data, cap_ * sizeof(*(vec).data)); \
            if (p) { \
                (vec).data = p; \
                (vec).capacity = cap_; \
                vec_f_push(vec, item); \
            } else { \
                VEC_OOM_HANDLER \
            } \
        } \
    } while (0)

#define vec_print_fmt(vec, fmt) \
    do { \
        size_t cou_ = (vec).count; \
        for(size_t i = 0; i<cou_; i++) { \
            printf(fmt"\n", (vec).data[i]); \
        } \
    } while (0)

#define vec_destroy(vec) \
    do { \
        if ((vec).data) { \
            free((vec).data); \
            (vec).data = NULL; \
        } \
        (vec).capacity = 0; \
        (vec).count = 0; \
    } while (0)

/* impotant : for now we don't resize even if we have No items in any big array */
#define vec_pop(vec) \
    (vec).count ? (vec).data + (--(vec).count) : NULL

/* could not make it with only evaulating idx once */
#define vec_get(vec, idx) \
    (vec).count > (size_t)(idx) && (size_t)(idx) ? (vec).data + (idx) : NULL


#endif /* MY_VEC_H */

