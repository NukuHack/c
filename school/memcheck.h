/* Memory check Header */

#if !defined(MEMCHECK_H)
#define MEMCHECK_H

#include "my_utils.h"

// main thing that should be used in other files
void check_mem_freed(void);

/* small checkers */
void ad_c_ptr_arr(void *ptr, size_t s);
size_t rm_c_ptr_arr(void *ptr);
size_t in_c_ptr_arr(void *ptr);
size_t hole_c_ptr_arr(void);

/* memory method wrappers */
void free_custom(void *ptr);
void *malloc_custom(size_t s);
void *calloc_custom(size_t z, size_t s);
void *realloc_custom(void *ptr, size_t s);

/* format a number */
char *fmt_num(LLU v, char *buf, size_t buflen);

#define malloc malloc_custom
#define free free_custom
#define calloc calloc_custom
#define realloc realloc_custom

#endif /* MEMCHECK_H */

