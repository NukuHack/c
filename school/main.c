#include "my_utils.h"
#include "my_vec.h"


typedef struct {
    size_t capacity;
    size_t count;
    double *data;
} DuoList;

typedef struct {
    size_t capacity;
    size_t count;
    int *data;
} IntList;

int main(void) {
    DuoList v = {0};
    vec_make(v, 20);

    vec_push(v, 2.4);
    vec_push(v, 1.4);
    vec_push(v, 2.4);
    vec_push(v, 3.4);

    vec_print_fmt(v, FMT_double);
    vec_destroy(v);

    IntList f = {0};
    vec_push(f, 2);
    vec_push(f, 4);
    vec_push(f, 1);
    int *x = vec_pop(f);
    printP(int, x);
    vec_push(f, 3);
    
    vec_print_fmt(f, FMT_int);
    vec_destroy(f);

#if defined(MEMCHECK)
    check_mem_freed();
#endif
    return 0;
}
