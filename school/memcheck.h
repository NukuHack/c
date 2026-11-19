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
void free_my_c_im(void *ptr);
void *malloc_my_c_im(size_t s);
// bruh do not use it to allocate more than 1.8e19 bytes of memory ...
// roughly 4Gigs for z and s
void *calloc_my_c_im(size_t z, size_t s);
void *realloc_my_c_im(void *ptr, size_t s);

/* format a number */
char *fmt_num(LLU v, char *buf, size_t buflen);

#define malloc malloc_my_c_im
#define free free_my_c_im
#define calloc calloc_my_c_im
#define realloc realloc_my_c_im

#endif /* MEMCHECK_H */

