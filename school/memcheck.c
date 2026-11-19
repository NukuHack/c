#include "memcheck.h" // self
#include "logging.h" // Logging

#undef malloc
#undef free
#undef calloc
#undef realloc

const char *const units[] = {"B","KB","MB","GB","TB","PB","EB"};

const size_t INVALID = (size_t)-1; // should be 0b1111 ... 111 (all 64 bit)
void *malloced_pointers[BUF_LEN] = {0};
size_t pointer_sizes[BUF_LEN] = {0};
size_t current_pointer = 0;


void check_mem_freed(void) {
    bool clean = true;
    for (size_t i = current_pointer; i-- > 0;) {
        if (malloced_pointers[i]) {
            char num[20]; fmt_num((LLU)pointer_sizes[i], ARR_L(num));
            log_error("check: not freed memory %p (%s)\n",
                    malloced_pointers[i], num);
            clean = false;
        }
    }
    if (clean) {
        log_debug("freed all memory nicely!");
    } else {
        log_error("memory trash remains!!");
    }
}


size_t hole_c_ptr_arr(void) {
    for (size_t i = 0; BUF_LEN>i; i++) {
        if (!malloced_pointers[i]) {
            return i;
        }
    }
    log_error("no free memory !!! atleast %d pointers used (entire storage is full)\n", BUF_LEN);
    abort();
    return INVALID;
}
size_t in_c_ptr_arr(void *ptr) {
    for (size_t i = current_pointer; i-- > 0;) {
        if (malloced_pointers[i] == ptr) {
            return pointer_sizes[i];
        }
    }
    return INVALID;
}
size_t rm_c_ptr_arr(void *ptr) {
    for (size_t i = current_pointer; i-- > 0;) {
        if (malloced_pointers[i] == ptr) {
            malloced_pointers[i] = NULL;
            return pointer_sizes[i];
        }
    }
    return INVALID;
}
void ad_c_ptr_arr(void *ptr, size_t size) {
    if (current_pointer > BUF_LEN / 2) {
        size_t i = hole_c_ptr_arr();
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
        log_error("free: NULL ....\n");
        return;
    }
    size_t o;
    if ((o = rm_c_ptr_arr(ptr)) == INVALID) {
        log_error("free: double freed memory : %p\n", ptr);
        return;
    }
    char num[20]; fmt_num((LLU)o, ARR_L(num));
    log_debug("free: done, %p removed from storage (%s)", ptr, num);
    free(ptr);
}
void *malloc_custom(size_t s) {
    void *ptr = (void*)malloc(s);
    char num[20]; fmt_num((LLU)s, ARR_L(num));
    if (!ptr) {
        log_error("malloc: could not malloc (%s)\n", num);
    } else {
        log_debug("malloc: added %p to storage (%s)\n", ptr, num);
        ad_c_ptr_arr(ptr, s);
    }
    return ptr;
}
void *calloc_custom(size_t z, size_t s) {
    size_t siz = z * s;
    char num[20]; fmt_num((LLU)siz, ARR_L(num));
    void *ptr = (void*)calloc(1, siz);
    if (!ptr) {
        log_error("calloc: could not calloc (%s)\n", num);
    } else {
        log_debug("calloc: added %p to storage (%s)\n", ptr, num);
        ad_c_ptr_arr(ptr, siz);
    }
    return ptr;
}
void *realloc_custom(void *ptr, size_t s) {
    size_t o;
    if (ptr && (o = rm_c_ptr_arr(ptr)) == INVALID) {
        log_error("realloc: realloc of ptr, not ever malloc-ed : %p\n", ptr);
        return NULL;
    }
    char num[20]; fmt_num((LLU)o, ARR_L(num));
    if (ptr)
        log_debug("realloc: removed %p from storage (%s)\n", ptr, num);
    void *saved;
    memcpy(&saved, &ptr, sizeof(void*)); // so gcc not complains about a "use after fee"
    void *pptr = (void*)realloc(ptr, s);
    char numS[20]; fmt_num((LLU)s, ARR_L(numS));
    if (!pptr) {
        log_error("realloc: could not realloc %p (%s) to (%s)\n", saved, num, numS);
        if (saved) {
            ad_c_ptr_arr(saved, o);
            log_debug("realloc: re-saved (original) %p to storage (%s)\n", saved, num);
        }
        return NULL;
    } else {
        log_debug("realloc: saved %p to storage (%s)\n", pptr, numS);
        ad_c_ptr_arr(pptr, s);
        return pptr;
    }
}


char *fmt_num(LLU v, char *buf, size_t buflen) {
    int MAX_TIER = ARR_S(units)-1; /* index of the max */

    if (!buf || buflen == 0)
        return buf;
    if (v < 1000ULL) {
        snprintf(buf, buflen, "%llu B", v);
        return buf;
    }

    /* 1. Find tier: d = 1000^tier, the largest power with v / d >= 1.
     *    1000^6 = 1e18 fits in 64 bits, and v/1e18 <= 18, so the loop
     *    can never run past EB. */
    unsigned long long d = 1ULL;
    int tier = 0;
    while (tier < MAX_TIER && v / d >= 1000ULL) { d *= 1000ULL; ++tier; }

    unsigned long long ip  = v / d;             /* integer part, 1..999 */
    unsigned long long rem = v % d;             /* remainder, < d <= 1e18 */

    /* 2. 5 significant digits total. */
    int decimals = 5 - ((ip >= 100) ? 3 : (ip >= 10) ? 2 : 1);

    /* 3. Exact long division for the fractional digits.
     *    rem * 10 < 1e19 < 2^64, so it never overflows. */
    unsigned frac = 0, pow10 = 1;
    for (int i = 0; i < decimals; ++i) {
        rem *= 10;
        frac = frac * 10 + (unsigned)(rem / d);
        rem %= d;
        pow10 *= 10;
    }

    /* 4. Round half up on the exact remainder, then propagate carries. */
    if (rem * 2 >= d && ++frac == pow10) {
        frac = 0;
        if (++ip == 1000ULL && tier < MAX_TIER) { ip = 1; ++tier; }
    }

    /* 5. Fraction digits, trailing zeros trimmed. */
    char dig[4];
    for (int i = decimals - 1; i >= 0; --i) { dig[i] = (char)('0' + frac % 10); frac /= 10; }
    while (decimals > 0 && dig[decimals - 1] == '0') --decimals;

    if (decimals)
        snprintf(buf, buflen, "%llu.%.*s %s", ip, decimals, dig, units[tier]);
    else
        snprintf(buf, buflen, "%llu %s", ip, units[tier]);
    return buf;
}

