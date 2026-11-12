#include "string.h" // self
#include <stdlib.h> // memory
#include "my_utils.h" // io

#ifndef DYN_STR
#define DYN_STR // we already have it imported ... might aswell use it
#endif

int string_main(void) {
    char* str = inp_str();
    if (!str) {
        err("could not read input");
        return 1;
    }
    print(str);
    free(str);

    return 0;
}

struct String {
    char *data;
    size_t len;
    size_t sz;
};

String* sv_create(void) {
    String* v = (String*)malloc(sizeof *v);
    if (!v) {
        err("malloc failed");
        return NULL;
    }
    v->data = NULL;
    v->len = 0;
    v->sz = 0;
    return v;
}

void sv_free(String* v) {
    if (!v) return;
    if (v->data)
        free(v->data);
    free(v);
}
char* sv_free_get(String* v) {
    if (!v) return NULL;
    char* d = v->data;
    free(v);
    return d;
}

bool sv_reserve(String *v, size_t need) {
    if (!v) return false;
    if (need <= v->sz) return true;

    size_t ncap = v->sz ? v->sz * 2 : 128;
    while (ncap < need) ncap <<= 1;

    char *p = (char*)realloc(v->data, ncap);
    if (!p) return false;

    v->data = p;
    v->sz = ncap;
    return true;
}

bool sv_push(String *v, char c) {
    if (!v || !sv_reserve(v, v->len + 1)) return false;
    v->data[v->len++] = c;
    return true;
}


