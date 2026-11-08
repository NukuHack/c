// self
#include "string.h"
// impl

#include "small_lib.h"


int string_main(void) {
    char* str = inp_str();
    free(str);

    return 0;
}

bool is_alpha(char inp) {
    return (inp >= 'a' && inp <= 'z') || (inp >= 'A' && inp <= 'Z');
}
bool is_numeric(char inp) {
    return inp >= '0' && inp <= '9';
}
bool is_alnum(char inp) {
    return is_numeric(inp) || is_alpha(inp);
}
char to_upper(char inp) {
    return (inp >= 'a' && inp <= 'z') ? inp - 32 : inp;
}
char to_lower(char inp) {
    return (inp >= 'A' && inp <= 'Z') ? inp + 32 : inp;
}

int str_len(const char* str) {
    if (!str) return 0;
    const char* p = str;
    while(*p) p++; // pointer walk
    return (int)(p-str);
}

char* alphabetic(char* a, char* b) {
    if (!a || !b || a == b)
        return NULL;

    int len1 = str_len(a);
    int len2 = str_len(b);
    int len = len1 > len2 ? len2 : len1;

    for (int i = 0; i<len; i++) {
        char ac = to_upper(a[i]);
        char bc = to_upper(b[i]);
        if (ac != bc)
            return ac > bc ? b : a;
    }

    return
        len < len1 ? b :
        len < len2 ? a :
        NULL;
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
    if (!v->data)
        return NULL;
    char* d = v->data;
    free(v);
    return d;
}

bool sv_reserve(String *v, size_t need) {
    if (!v) return false;
    if (need <= v->sz) return true;

    size_t ncap = v->sz ? v->sz * 2 : 128;
    while (ncap < need) ncap *= 2;

    char *p = realloc(v->data, ncap);
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


