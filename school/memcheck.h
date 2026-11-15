/* Memory check Header */

#if !defined(MEMCHECK_H)
#define MEMCHECK_H

#include "my_utils.h"

void ad_c_ptr_arr(void *ptr, size_t s);
#if ZERO
size_t in_c_ptr_arr(void *ptr);
#endif
size_t rm_c_ptr_arr(void *ptr);
void check_ptr_arr(void);
size_t check_ptr_arr_emp(void);

void free_custom(void *ptr);
void *malloc_custom(size_t s);
void *calloc_custom(size_t z, size_t s);
void *realloc_custom(void *ptr, size_t s);

#define malloc malloc_custom
#define free free_custom
#define calloc calloc_custom
#define realloc realloc_custom

#endif /* MEMCHECK_H */

