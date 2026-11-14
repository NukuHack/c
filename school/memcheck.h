/* Memory check Header */

#if !defined(MEMCHECK_H)
#define MEMCHECK_H

#if !defined(PTR_ARR_SIZ)
#   define PTR_ARR_SIZ 1024
#endif /* PTR_ARR_SIZ */

#include <stddef.h> // size_t
#include <stdio.h> // mem checking
#include <stdbool.h> // bool
#include <stdlib.h> // the real functions

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

