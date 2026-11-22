/* rusty.h — a thin "Rust-flavoured" skin over C.
 * Requires C99+ (C11 recommended).  `let`/`mut` need GCC or Clang.
 */
#ifndef RUSTY_H
#define RUSTY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ------------------------------------------------------------------ */
/*  Primitives                                                         */
/* ------------------------------------------------------------------ */

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef intptr_t  isize;   /* pointer-sized signed   */
typedef uintptr_t usize;   /* pointer-sized unsigned */

typedef float    f32;
typedef double   f64;

/* formatting */
#include <inttypes.h>

#define FMT_i8   "%" PRId8
#define FMT_i16  "%" PRId16
#define FMT_i32  "%" PRId32
#define FMT_i64  "%" PRId64
#define FMT_u8   "%" PRIu8
#define FMT_u16  "%" PRIu16
#define FMT_u32  "%" PRIu32
#define FMT_u64  "%" PRIu64
#define FMT_isize "%" PRIdPTR
#define FMT_usize "%" PRIuPTR

/* 128-bit ints, where the compiler provides them */
#if defined(__SIZEOF_INT128__)
typedef __int128          i128;
typedef unsigned __int128 u128;
#endif

/* bool / true / false come from <stdbool.h> */

/* ------------------------------------------------------------------ */
/*  Strings                                                            */
/* ------------------------------------------------------------------ */

typedef char       *String;   /* owned / mutable, like Rust's String  */
typedef const char *str;      /* borrowed, like &str                  */

/* ------------------------------------------------------------------ */
/*  Sugar                                                              */
/* ------------------------------------------------------------------ */

#if defined(__GNUC__) || defined(__clang__)
#  define let  __auto_type
#  define mut  /* no-op */
#  define var  __auto_type
#endif

/* C23 __VA_OPT__ vs. the GNU comma-swallowing extension */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#  define RUSTY_VA(...) __VA_OPT__(,) __VA_ARGS__
#else
#  define RUSTY_VA(...) , ##__VA_ARGS__
#endif

/* println!("fmt", ...) — appends a newline.  Format string must be a literal. */
#define println(fmt, ...)  printf(fmt "\n" RUSTY_VA(__VA_ARGS__))
#define print(fmt, ...)    printf(fmt RUSTY_VA(__VA_ARGS__))
#define eprintln(fmt, ...) fprintf(stderr, fmt "\n" RUSTY_VA(__VA_ARGS__))

#define panic(msg)     do {                                       \
        fprintf(stderr, "panic: %s (%s:%d)\n",                    \
                (msg), __FILE__, __LINE__);                       \
        abort();                                                  \
    } while (0)

#define todo()         do {                                       \
        fprintf(stderr, "not yet implemented (%s:%d)\n",          \
                __FILE__, __LINE__);                              \
        abort();                                                  \
    } while (0)

#define unreachable()  do {                                       \
        fprintf(stderr, "unreachable (%s:%d)\n",                  \
                __FILE__, __LINE__);                              \
        abort();                                                  \
    } while (0)

#endif /* RUSTY_H */
