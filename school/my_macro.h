
#if !defined(MY_MACRO)
#define MY_MACRO


#ifndef ZERO
// so i have sintax highlight in "if 0"
#  define ZERO 0
#endif /* ZERO */

#if !defined(BUF_LEN)
#     define BUF_LEN 128
#endif /* BUF_LEN */

// only destroys pointers, files need different handling
#if !defined(destroy) && !defined(destroy_arr)
#    define destroy(a) \
      do { \
        void *arr__ = (void*)(a); \
        if(arr__) \
            free(arr__); \
        arr__=NULL; \
      } while (0)
#    define destroy_arr(a, len) \
      do { \
        void **arr_ = (void**)(a); /* this is theoretically undefined-behaviour but for now it's fine */ \
        size_t count_ = (len); \
        if (arr_ && count_ > 0) { \
            for(size_t i = 0; i<count_; i++) \
                destroy(arr_[i]); \
            free(arr_); \
        } \
        arr_=NULL; \
      }  while (0)
#endif /* destroy */

// array size + length 
#if !defined(ARR_L) && !defined(ARR_S)
#  define ARR_L(x) x, ARR_S(x) /* basic */
#  define ARR_LM(x) x, ARR_S(x)-1 /* if we reserve one last for nil */
#  define ARR_Str(x) x, strlen(x) /* basic string */
#  define ARR_S(x) (sizeof(x)/sizeof(x[0])) /* just size of stack alloc-ed arr */
#endif /* ARR_L and ARR_S */

// evaulates values multiple times, careful with the calling
#if !defined(CLAMP)
#  define CLAMP(a, b, ty) (ty)MIN((a),(b)) // clamp a to b, return min, in type ty
#endif
#if !defined(MIN) && !defined(MAX) && !defined(ABS)
#  define MIN(a, b) ((a) < (b) ? (a) : (b))
#  define MAX(a, b) ((a) > (b) ? (a) : (b))
#  define ABS(a) ((a) < 0 ? -(a) : (a))
#  define MIN_D(a, b) MIN(*a, *b)
#  define MAX_D(a, b) MAX(*a, *b)
#endif /* MIN-MAX-ABS */

#if !defined(LLU) && !defined(LL) && !defined(LU)
#  define ULL long long unsigned int
#  define LLU ULL
#  define LL long long int
#  define UL long unsigned int
#  define LU UL
// basic :
#  define llong long long
#  define ulong UL
#  define ullong ULL
#endif /* LLU-LL-LU */

#if !defined(UINT_TYPE) && !defined(uint)
#  define UINT_TYPE
   typedef unsigned int uint;
#  define uint uint
#endif /* UINT_TYPE */

#if !defined(STR_TYPE) && !defined(str)
#  define STR_TYPE
   // since preprocessor does not like "char*"
   typedef char *str; // "define" does not work either, since that get "transformed" back
   typedef const char *cstr;
#  define str str
#  define cstr cstr
#endif /* STR_TYPE */

#if !defined(KB) && !defined(MB) && !defined(GB) // rest not listed
extern const char *const units[]; // in code
// yes i know they "should be" called like "kibibyte" for 1024 multipliers
// but usually they are called "kilobyte" even if those should be just powers of 10 
// #define B 1L        //  byte      2**0
#  define KB 1024ULL   // kilobyte   2**10
#  define MB (KB * KB) // megabyte   2**20
#  define GB (MB * KB) // gigabyte   2**30
#  define TB (GB * KB) // terrabyte  2**40
#  define PB (TB * KB) // petabyte   2**50
#  define EB (PB * KB) // exabyte    2**60
#  define ZB (EB * KB) // zettabyte  2**70
#  define YB (ZB * KB) // yottabyte  2**80
#  define RB (YB * KB) // ronnabyte  2**90
#  define QB (RB * KB) // qettabyte  2**100
// btw, might overflow some systems, this should be in a 128-bit u-int
#endif


/* swap the data of 2 pointers */
#define mem_swap(a, b, ty) \
  do { \
    ty Tmp; size_t s = sizeof(ty); \
    memcpy(&Tmp, (a), s); \
    memcpy((a), (b), s); \
    memcpy((b), &Tmp, s); \
  } while(0)


// even operations can be treated as "tokens" soo nice
#define Strcmp(A, o, B) (strcmp((A), (B)) o 0)

// auto cast memory alloc
#define New(ty, n) ( (ty *) malloc((n) * sizeof(ty)) )
#define New_c(ty, n) ( (ty *) calloc((n), sizeof(ty)) )
#define Re_New(ty, old, n) ( (ty *) realloc((old), (n) * sizeof(ty)) )

//  "cap" is the current capacity, by default it doubles
//  "NULL" = failed, anything else is a prt caller must free
#if ZERO // original function, before macro
void *resize_buff(void *buf, size_t *cap, size_t size) {
    if (!cap || size == 0 || size > GB || *cap > GB) {
        errno = EINVAL;
        return NULL;
    } /* after this check it will NOT overflow in 64 bit systems */

    size_t new_cap = *cap ? *cap * 2 : BUF_LEN;
    /*if (*cap * size > SIZE_MAX >> 1) {
        errno = ENOMEM;
        return NULL;
    }*/
    void *p = realloc(buf, new_cap * size);
    if (!p) return NULL;
    *cap = new_cap;
    return p;
}
#endif
#define resize_buff(out, buf, cap, size) \
    do { \
        size_t *cap_p = (cap),  cap_,  size_, new_cap; \
        void *buf_ = (buf); \
        if (!cap_p || !buf_ || (size_ = (size)) == 0 || size_ > GB || (cap_ = *cap_p) > GB) { \
            errno = EINVAL; \
            (out) = NULL; \
            break; \
        } /* after this check it will NOT overflow in 64 bit systems */ \
        new_cap = cap_ ? cap_ * 2 : BUF_LEN; \
        (out) = realloc(buf, new_cap * size_); \
        if (!(out)) { \
            errno = ENOMEM; \
            (out) = NULL; \
            break; \
        } \
        *(cap) = new_cap; \
    } while (0)

// never frees, dealloc must be handled by the user
#define TRY_GROW(buf, cap, size, out_ok_) \
    do { \
        void *buf_ptr_ = (buf), *ptr; \
        size_t temp_cap_ = *(cap),  elem_size_ = (size); \
        resize_buff(ptr, buf_ptr_, &temp_cap_, elem_size_); \
        if (ptr) { \
            (buf) = ptr; \
            *(cap) = temp_cap_; \
            (out_ok_) = true; \
        } else { \
            (out_ok_) = false; \
        } \
    } while (0)

#define IF_GROW(buf, cap, count, ty) \
    bool tested_bool_at_grow_; /* this is exposed outside so only one 'IF_GROW' can be used in one scope */ \
    do { /* inner scope not to expose the rest of the variables */ \
        size_t current_size_ = *(cap); \
        size_t current_count_ = (count); \
        if (current_size_ < 4u || current_size_ < current_count_ + 3u) { \
            void *buf_ptr_ = (buf), *p; \
            resize_buff(p, buf_ptr_, &current_size_, sizeof(ty)); \
            if (p) { \
                (buf) = p; \
                *(cap) = current_size_; \
                tested_bool_at_grow_ = true; \
            } else { \
                tested_bool_at_grow_ = false; \
            } \
        } \
    } while (0); \
    if (!tested_bool_at_grow_) /* so after the macro we can do out custom handling */



#define ERRNO_FMT "errno= %d (\"%s\")"
#define ERRNO_ARG errno, strerror(errno)
#define printERR fprintf(stderr, ERRNO_FMT, ERRNO_ARG)


#if !defined(IO)
#  define IO stdin,stdout
#endif /* IO */


// Basic string concat .. yet sooo powerful
#define CAT_I(a, b) a##b
#define CAT(a, b) CAT_I(a, b)

/* bool */
#define NOT(c) CAT(NOT_, c)
#define NOT_0 1
#define NOT_1 0

// this is insainly unsafe - what if the result is not 0 or 1 ...
#define IF(cond, t, f) CAT(IF_, cond)(t, f)
#define IF_0(t, f) f
#define IF_1(t, f) t
#define IF_T(cond, t) IF(cond, t, (void)0)
#define IF_F(cond, f) IF(cond, (void)0, f)



#define FMT_int "%d"
#define FMT_long "%ld"
#define FMT_llong "%lld"
#define FMT_double "%f"
#define FMT_char "%c"
#define FMT_str "%s"
#define FMT_cstr FMT_str
#define FMT_uint "%u"
#define FMT_ulong "%lu"
#define FMT_ullong "%llu"
#define FMT_float FMT_double

#define GET_int inpi()
// TODO : long & llong & ulong & ullong
#define GET_long (long)inpi()
#define GET_llong (LL)inpi()
#define GET_uint inpu()
#define GET_ulong (UL)inpu()
#define GET_ullong (ULL)inpu()
#define GET_double inpd()
#define GET_char inpc()
#define GET_str inps()
#define GET_cstr inps()
// TODO : float
#define GET_float (float)inpd()

// Compile time switch !!!!!!
// since this is a macro, but just a concat, it is really easy to extend to anything new from other files
//   could make it safer, with default fallback (here it could be just an error print at compiletime)
//   but that would need much more checking prop. __VA_ARGS__ so pain ...
#define SWITCH(cond) CAT(SWITCH_, cond)
#define SWITCH_FMT(ty) CAT(FMT_, ty)
#define SWITCH_GET(ty) CAT(GET_, ty)

// "high level wrappers"
#define printF(ty, val) printf(SWITCH_FMT(ty) "\n", val)
#define VAR(ty, name, val) ty name = val;
#define GET(ty, name) VAR(ty, name, SWITCH_GET(ty))



// "array" macros
#define arr_reverse(Ty, arr, len) \
    do { \
        size_t len_ = (size_t)(len); \
        Ty *ar_ = (arr); \
        if (ar_ != NULL && len_ >= 2u) \
        for (size_t i = 0; i < len_ / 2u; i++) { \
            Ty tmp = ar_[i]; \
            ar_[i] = ar_[len_ - i - 1u]; \
            ar_[len_ - i - 1u] = tmp; \
        } \
    } while (0)

// only one foreach, can't put one inside another (like 2d foreach)
#define arr_foreach(Ty, var, arr, len) \
    for (Ty var, *ar_s_ = (Ty *)(arr), *ar_p_ = ar_s_, \
         *ar_e_ = ar_s_ ? ar_s_ + (size_t)(len) : ar_s_; \
         ar_p_ != ar_e_ && ((var = *ar_p_), 1); ar_p_++)

/* Current index; valid only inside an arr_foreach body. */
#define arr_for_i ((size_t)(ar_p_ - ar_s_))

#define arr_foreach_p(Ty, var, arr, len) \
    for (Ty *var = (arr), *ar_e_ = var ? var + (size_t)(len) : var; var != ar_e_; var++)


#define arr_print(ty, arr, len) \
    arr_foreach(ty, var, (arr), (len)) \
        printf("idx %zu) : "SWITCH_FMT(ty)"\n", arr_for_i, var);


#endif /* MY_MACRO */

