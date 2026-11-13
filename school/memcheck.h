/* Memory check Header */

#if !defined(MEMCHECK_H)

#if !defined(PTR_ARR_SIZ)
#   define PTR_ARR_SIZ 1024
#endif /* PTR_ARR_SIZ */

#include <stddef.h> // size_t
#include <stdio.h> // mem checking
#include <stdbool.h> // bool
#include <stdlib.h> // the real functions
#include "logging.h" // Logging

#endif /* MEMCHECK_H */



#if defined(MY_MEMCHECK_IMPL)

void *malloced_pointers[PTR_ARR_SIZ] = {0};
size_t current_pointer;

void check_ptr_arr(void) {
    for (size_t i = current_pointer; i-- > 0;) {
        if (malloced_pointers[i]) {
            LOG_ERROR("check: not freed memory : %p\n", malloced_pointers[i]);
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
bool in_c_ptr_arr(void *ptr) {
    for (size_t i = current_pointer; i-- > 0;) {
        if (malloced_pointers[i] == ptr) {
            return true;
        }
    }
    return false;
}
bool rm_c_ptr_arr(void *ptr) {
    for (size_t i = current_pointer; i-- > 0;) {
        if (malloced_pointers[i] == ptr) {
            malloced_pointers[i] = NULL;
            return true;
        }
    }
    return false;
}
void ad_c_ptr_arr(void *ptr) {
    if (current_pointer > PTR_ARR_SIZ / 2) {
        size_t space = check_ptr_arr_emp();
        if (space > current_pointer)
            current_pointer = space;
        malloced_pointers[space] = ptr;
    } else
        malloced_pointers[current_pointer++] = ptr;
}



void free_custom(void *ptr) {
    if (!ptr) { 
        LOG_ERROR("free: NULL ....\n");
    } else {        
        if (!rm_c_ptr_arr(ptr)) {
            LOG_ERROR("free: double freed memory : %p \n", ptr);
            return;
        }
        LOG_DEBUG("free: done, %p removed from storage", ptr);
        free(ptr);
    }
}
void *malloc_custom(size_t s) {
    void *ptr = (void*)malloc(s);
    if (!ptr) {
        LOG_ERROR("malloc: could not malloc %lu bytes \n", s);
    } else {
        LOG_DEBUG("malloc: added %p to storage\n", ptr);
        ad_c_ptr_arr(ptr);
    }
    return ptr;
}
void *calloc_custom(size_t z, size_t s) {
    void *ptr = (void*)calloc(z, s);
    if (!ptr) {
        LOG_ERROR("calloc: could not calloc %lu * %lu bytes \n", z, s);
    } else {
        LOG_DEBUG("calloc: added %p to storage\n", ptr);
        ad_c_ptr_arr(ptr);
    }
    return ptr;
}
void *realloc_custom(void *ptr, size_t s) {
    if (!ptr) {
        LOG_WARN("realloc: null ...");
        return malloc_custom(s);
    }
    if (s == 0) {
        LOG_WARN("realloc: free with realloc");
        free_custom(ptr);
        return NULL;
    }
    if (!rm_c_ptr_arr(ptr)) {
        LOG_ERROR("realloc: realloc of ptr, not ever malloc-ed : %p \n", ptr);
        return NULL;
    }
    LOG_DEBUG("realloc: removed %p from storage \n", ptr);
    void *pptr = (void*)realloc(ptr, s);
    if (!pptr) {
        LOG_ERROR("realloc: could not realloc %p to %lu bytes \n", ptr, s);
        ad_c_ptr_arr(ptr);
        LOG_DEBUG("realloc: re-saved (original) %p to storage\n", ptr);
        return NULL;
    } else {
        LOG_DEBUG("realloc: saved %p to storage\n", ptr, pptr);
        ad_c_ptr_arr(pptr);
        return pptr;
    }
}
#undef MY_MEMCHECK_IMPL
#endif /* MY_MEMCHECK_IMPL */


#if !defined(MEMCHECK_H)
#define MEMCHECK_H

extern void *malloced_pointers[PTR_ARR_SIZ];

void ad_c_ptr_arr(void *ptr);
bool in_c_ptr_arr(void *ptr);
bool rm_c_ptr_arr(void *ptr);
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

