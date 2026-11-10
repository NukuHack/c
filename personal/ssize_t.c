/* portable_ssize_t.h — conservative portable ssize_t
 *
 * Strategy (in order):
 *   1. If the target looks POSIX‑like (or has _POSIX_VERSION / _POSIX_C_SOURCE),
 *      do NOTHING.  The user must include <sys/types.h> themselves.
 *   2. If any known system guard is already defined (glibc, musl, FreeBSD,
 *      newlib, MinGW, Windows SDK, or a project‑wide HAVE_SSIZE_T), do NOTHING.
 *      This is the “do no harm” path.
 *   3. On Windows, use SSIZE_T from <basetsd.h> (or the compiler’s __int64/int)
 *      and define the Windows guards so later headers don’t redefine it.
 *   4. On GCC / Clang, use the compiler’s __SIZEOF_* / __SIZE_MAX__ builtins to
 *      pick the signed integer type with the same width as size_t.
 *   5. Otherwise (unknown compiler, non‑POSIX, non‑Windows) define it as
 *      ptrdiff_t — the only standard signed size‑like type — but only if the
 *      caller has not explicitly set SSIZE_T_NO_FALLBACK.
 *
 * The header deliberately does NOT include <sys/types.h> on POSIX systems,
 * so it cannot accidentally hide a platform‑specific ssize_t.
 */
#ifndef PORTABLE_SSIZE_T_H
#define PORTABLE_SSIZE_T_H

#include <stddef.h>   /* ptrdiff_t, size_t */
#include <limits.h>   /* SSIZE_MAX (if the implementation provides it) */

/* ------------------------------------------------------------------ */
/* 1. POSIX‑like detection: instant skip                               */
/* ------------------------------------------------------------------ */
#if defined(__unix__) || defined(__unix) || defined(unix) \
 || defined(__linux__) || defined(__linux)                \
 || defined(__APPLE__) || defined(__MACH__)               \
 || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__) \
 || defined(__CYGWIN__)                                   \
 || defined(__ANDROID__)                                  \
 || defined(__EMSCRIPTEN__)                               \
 || defined(__sun) || defined(__solaris__)                \
 || defined(__hpux) || defined(_AIX) || defined(__osf__) || defined(__sgi) || defined(__ultrix) \
 || defined(__QNX__)                                      \
 || defined(_POSIX_VERSION) || defined(_POSIX_C_SOURCE)
#  define PORTABLE_SSIZE_T_IS_POSIX 1
#endif

/* ------------------------------------------------------------------ */
/* 2. Known system guards: if any is set, ssize_t already exists       */
/* ------------------------------------------------------------------ */
#if defined(_SSIZE_T_DEFINED) || defined(_SSIZE_T_DEFINED_) \
 || defined(_SSIZE_T) || defined(_SSIZE_T_)                 \
 || defined(_SSIZE_T_DECLARED)                             \
 || defined(__ssize_t_defined)   /* glibc */                \
 || defined(__DEFINED_ssize_t)   /* musl  */                \
 || defined(HAVE_SSIZE_T)        /* config‑style */         \
 || defined(SSIZE_MAX)           /* <limits.h> only defines it if ssize_t exists */
#  define PORTABLE_SSIZE_T_ALREADY_DEFINED 1
#endif

/* ------------------------------------------------------------------ */
/* 3. Define only when we have high confidence it is missing           */
/* ------------------------------------------------------------------ */
#if !defined(PORTABLE_SSIZE_T_IS_POSIX) && !defined(PORTABLE_SSIZE_T_ALREADY_DEFINED)

#  if defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER)

     /* --- Windows (MSVC, MinGW, clang‑cl, etc.) ---------------------- */
     /* <basetsd.h> is part of every Windows SDK and MinGW. */
#    if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
#      include <basetsd.h>
       typedef SSIZE_T ssize_t;
#    elif defined(_WIN64)
       typedef __int64 ssize_t;   /* MSVC without basetsd.h (unlikely) */
#    else
       typedef int ssize_t;
#    endif

     /* Tell later Windows/MinGW headers we already provided it. */
#    define _SSIZE_T_DEFINED
#    define _SSIZE_T_DEFINED_
#    define _SSIZE_T
#    define _SSIZE_T_
#    define _SSIZE_T_DECLARED
#    define __ssize_t_defined
#    define __DEFINED_ssize_t

#  elif defined(__GNUC__) || defined(__clang__)

     /* --- GCC / Clang (but not on a POSIX OS) ------------------------ */
     /* Prefer a builtin size comparison; __SIZEOF_SIZE_T__ is available
        on GCC ≥ 4.4 and Clang ≥ 3.8.  As a last resort, compare __SIZE_MAX__
        against the signed maximums. */
#    if defined(__SIZEOF_SIZE_T__) && defined(__SIZEOF_INT__) \
     && (__SIZEOF_SIZE_T__ == __SIZEOF_INT__)
       typedef int ssize_t;
#    elif defined(__SIZEOF_SIZE_T__) && defined(__SIZEOF_LONG__) \
       && (__SIZEOF_SIZE_T__ == __SIZEOF_LONG__)
       typedef long ssize_t;
#    elif defined(__SIZEOF_SIZE_T__) && defined(__SIZEOF_LONG_LONG__) \
       && (__SIZEOF_SIZE_T__ == __SIZEOF_LONG_LONG__)
       typedef long long ssize_t;
#    elif defined(__SIZEOF_SIZE_T__) && defined(__SIZEOF_PTRDIFF_T__) \
       && (__SIZEOF_SIZE_T__ == __SIZEOF_PTRDIFF_T__)
       typedef ptrdiff_t ssize_t;   /* ptrdiff_t has the same width */
#    elif defined(__SIZE_MAX__) && defined(__INT_MAX__) \
       && ((__SIZE_MAX__) / 2 == (__INT_MAX__))
       typedef int ssize_t;
#    elif defined(__SIZE_MAX__) && defined(__LONG_MAX__) \
       && ((__SIZE_MAX__) / 2 == (__LONG_MAX__))
       typedef long ssize_t;
#    elif defined(__SIZE_MAX__) && defined(__LONG_LONG_MAX__) \
       && ((__SIZE_MAX__) / 2 == (__LONG_LONG_MAX__))
       typedef long long ssize_t;
#    else
#      error "Cannot determine a signed type for ssize_t on this GCC/Clang target"
#    endif

     /* Prevent any later header from redefining it. */
#    define _SSIZE_T_DEFINED
#    define _SSIZE_T_DEFINED_
#    define _SSIZE_T
#    define _SSIZE_T_
#    define _SSIZE_T_DECLARED
#    define __ssize_t_defined
#    define __DEFINED_ssize_t

#  else

     /* --- Unknown compiler, non‑POSIX, non‑Windows ------------------- */
     /* ptrdiff_t is the only standard signed “size‑like” type.
        On every mainstream flat‑memory target its width equals size_t’s.
        If you are on an exotic segmented architecture, define
        SSIZE_T_NO_FALLBACK and supply your own ssize_t. */
#    if !defined(SSIZE_T_NO_FALLBACK)
       typedef ptrdiff_t ssize_t;

#      define _SSIZE_T_DEFINED
#      define _SSIZE_T_DEFINED_
#      define _SSIZE_T
#      define _SSIZE_T_
#      define _SSIZE_T_DECLARED
#      define __ssize_t_defined
#      define __DEFINED_ssize_t
#    endif

#  endif /* Windows / GCC‑Clang / plain fallback */

#endif /* not POSIX and not already defined */

#endif /* PORTABLE_SSIZE_T_H */