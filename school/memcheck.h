/* Memory check Header */

#if !defined(MEMCHECK_H)

#if !defined(PTR_ARR_SIZ)
#   define PTR_ARR_SIZ 1024
#endif /* PTR_ARR_SIZ */

#include <stddef.h> // size_t
#include <stdio.h> // mem checking
#include <stdbool.h> // bool
#include <stdlib.h> // the real functions
#include <string.h> // mem op-s
#include "logging.h" // Logging

#endif /* MEMCHECK_H */



#if defined(MY_MEMCHECK_IMPL)

void *malloced_pointers[PTR_ARR_SIZ] = {0};
size_t pointer_sizes[PTR_ARR_SIZ] = {0};
size_t current_pointer;

void check_ptr_arr(void) {
    for (size_t i = current_pointer; i-- > 0;) {
        if (malloced_pointers[i]) {
            LOG_ERROR("check: not freed memory, size %lu : %p\n",
                    malloced_pointers[i], pointer_sizes[i]);
        }
    }
}
size_t check_ptr_arr_emp(void) {
    for (size_t i = 0; PTR_ARR_SIZ>i; i++) {
        if (!malloced_pointers[i]) {
            return i;
        }
    }
    LOG_ERROR("no free memory !!! atleast %d pointers used (entire storage is full)\n", PTR_ARR_SIZ);
    abort();
    return 0;
}
size_t in_c_ptr_arr(void *ptr) {
    for (size_t i = current_pointer; i-- > 0;) {
        if (malloced_pointers[i] == ptr) {
            return pointer_sizes[i];
        }
    }
    return 0;
}
size_t rm_c_ptr_arr(void *ptr) {
    for (size_t i = current_pointer; i-- > 0;) {
        if (malloced_pointers[i] == ptr) {
            malloced_pointers[i] = NULL;
            return pointer_sizes[i];
        }
    }
    return 0;
}
void ad_c_ptr_arr(void *ptr, size_t size) {
    if (current_pointer > PTR_ARR_SIZ / 2) {
        size_t i = check_ptr_arr_emp();
        if (i > current_pointer)
            current_pointer = i;
        malloced_pointers[i] = ptr;
        pointer_sizes[i] = size;
    } else {
        malloced_pointers[current_pointer] = ptr;
        pointer_sizes[current_pointer++] = size;
    }
}



void free_custom(void *ptr) {
    if (!ptr) { 
        LOG_ERROR("free: NULL ....\n");
    } else {
        size_t o = 0;
        if ((o = rm_c_ptr_arr(ptr)) == 0) {
            LOG_ERROR("free: double freed memory : %p \n", ptr);
            return;
        }
        LOG_DEBUG("free: done, %p removed (%lu bytes) from storage", ptr, o);
        free(ptr);
    }
}
void *malloc_custom(size_t s) {
    if (s == 0) {
        LOG_ERROR("malloc: tried to malloc 0 bytes\n");
        return NULL;
    }
    void *ptr = (void*)malloc(s);
    if (!ptr) {
        LOG_ERROR("malloc: could not malloc %lu bytes \n", s);
    } else {
        LOG_DEBUG("malloc: added %p to storage (size %lu)\n", ptr, s);
        ad_c_ptr_arr(ptr, s);
    }
    return ptr;
}
void *calloc_custom(size_t z, size_t s) {
    size_t siz = z * s;
    if (siz == 0) {
        LOG_ERROR("calloc: tried to calloc 0 bytes\n");
        return NULL;
    }
    void *ptr = (void*)calloc(1, siz);
    if (!ptr) {
        LOG_ERROR("calloc: could not calloc %lu bytes \n", siz);
    } else {
        LOG_DEBUG("calloc: added %p to storage (size %lu)\n", ptr, siz);
        ad_c_ptr_arr(ptr, siz);
    }
    return ptr;
}
void *realloc_custom(void *ptr, size_t s) {
    size_t o = 0;
    if (ptr && (o = rm_c_ptr_arr(ptr)) == 0) {
        LOG_ERROR("realloc: realloc of ptr, not ever malloc-ed : %p \n", ptr);
        return NULL;
    }
    if (ptr)
        LOG_DEBUG("realloc: removed %p (%lu bytes) from storage \n", ptr, o);
    void *saved;
    memcpy(&saved, &ptr, sizeof(void*));
    void *pptr = (void*)realloc(ptr, s);
    if (!pptr) {
        LOG_ERROR("realloc: could not realloc %p (%lu bytes) to %lu bytes \n", saved, o, s);
        if (saved) {
            ad_c_ptr_arr(saved, o);
            LOG_DEBUG("realloc: re-saved (original) %p (%lu bytes) to storage\n", saved, o);
        }
        return NULL;
    } else {
        if (s > 0) {
            LOG_DEBUG("realloc: saved %p to storage (size %lu)\n", pptr, s);
            ad_c_ptr_arr(pptr, s);
        } else {
            LOG_DEBUG("realloc: freed %p (%lu bytes)\n", saved, o);
        }
        return pptr;
    }
}
#undef MY_MEMCHECK_IMPL
#endif /* MY_MEMCHECK_IMPL */


#if !defined(MEMCHECK_H)
#define MEMCHECK_H

extern void *malloced_pointers[PTR_ARR_SIZ];

void ad_c_ptr_arr(void *ptr, size_t s);
size_t in_c_ptr_arr(void *ptr);
size_t rm_c_ptr_arr(void *ptr);
void check_ptr_arr(void);
size_t check_ptr_arr_emp(void);

void free_custom(void *ptr);
void *malloc_custom(size_t s);
void *calloc_custom(size_t z, size_t s);
void *realloc_custom(void *ptr, size_t s);

#define malloc(x) malloc_custom(x)
#define free(x) free_custom(x)
#define calloc(y, x) calloc_custom(y, x)
#define realloc(y, x) realloc_custom(y, x)

#endif /* MEMCHECK_H */

