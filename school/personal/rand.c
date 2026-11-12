#include "rand.h"

#include <math.h>
#include <stdint.h>

#define BIG_9E3  0x9E3779B97F4A7C15ULL
#define BIG_BF5  0xBF58476D1CE4E5B9ULL
#define BIG_940  0x94D049BB133111EBULL

static uint64_t rng_s[2] = { BIG_9E3, BIG_BF5 };

static inline uint64_t splitmix64(uint64_t *x) {
    uint64_t z = (*x += BIG_9E3);
    z = (z ^ (z >> 30)) * BIG_BF5;
    z = (z ^ (z >> 27)) * BIG_940;
    return z ^ (z >> 31);
}

void seed_rng(uint64_t seed) {
    rng_s[0] = splitmix64(&seed);
    rng_s[1] = splitmix64(&seed);
    if (rng_s[0] == 0 && rng_s[1] == 0) rng_s[0] = 1;
}

static inline uint64_t rotl64(uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

/* xoroshiro128++ — same state, faster/better low-bits than xorshift128+ */
static inline uint64_t next_u64(void) {
    const uint64_t s0 = rng_s[0];
    uint64_t s1 = rng_s[1];
    const uint64_t result = rotl64(s0 + s1, 17) + s0;
    s1 ^= s0;
    rng_s[0] = rotl64(s0, 49) ^ s1 ^ (s1 << 21);
    rng_s[1] = rotl64(s1, 28);
    return result;
}

uint64_t randu64(void) { return next_u64(); }

/* Uniform in [0, range), range >= 1 — Lemire's nearly-divisionless method.
 * Outer loop is essentially never entered for small ranges. */
static inline uint64_t bounded_u64(uint64_t range) {
#if defined(__SIZEOF_INT128__)
    __uint128_t m = (__uint128_t)next_u64() * range;
    uint64_t l = (uint64_t)m;
    if (l < range) {
        uint64_t t = (uint64_t)(-range) % range;
        while (l < t) {
            m = (__uint128_t)next_u64() * range;
            l = (uint64_t)m;
        }
    }
    return (uint64_t)(m >> 64);
#else
    uint64_t t = (uint64_t)(-range) % range;
    uint64_t r;
    do { r = next_u64(); } while (r < t);
    return r % range;
#endif
}

/* ================================================================
 * Macro-generated integer generators
 *
 * Full range: take the top 8*sizeof(T) bits of a u64.  The shift is
 * a compile-time constant: 0 for 8-byte T, 32 for 4-byte T, 56 for
 * 1-byte T, etc.  Going through (UT) makes the signed conversion
 * well-defined for every bit pattern.
 *
 * Ranged: the span is computed in UT (the unsigned counterpart).
 * span == 0 in UT arithmetic means "the whole 2^(8*sizeof(T))
 * range" — detect it, and either use a raw u64 (8-byte T) or
 * bound on 2^(8*sizeof(T)) (< 8-byte T).  The `(1 << (bits-1)) * 2`
 * trick writes 2^bits without ever shifting by 64 (avoids UB).
 *
 * The macros assume sizeof(T) <= 8 — a 128-bit type would need a
 * proper 128-bit path.
 * ================================================================ */

#define DEFINE_RAND_FULL(NAME, T, UT) \
    T NAME(void) { \
        return (T)(UT)(next_u64() >> (64 - 8 * sizeof(T))); \
    }

#define DEFINE_RAND_RANGE(NAME, T, UT) \
    T NAME(T min, T max) { \
        if (min == max) return min; \
        if (min > max) { T _t = min; min = max; max = _t; } \
        UT _span = (UT)max - (UT)min + 1; \
        uint64_t _r; \
        if (_span) { \
            _r = bounded_u64((uint64_t)_span); \
        } else if (sizeof(T) >= 8) { \
            _r = next_u64(); \
        } else { \
            _r = bounded_u64(((uint64_t)1 << (8 * sizeof(T) - 1)) * 2); \
        } \
        return (T)((UT)min + (UT)_r); \
    }

DEFINE_RAND_FULL (randi,    int,                unsigned)
DEFINE_RAND_RANGE(randi_r,  int,                unsigned)
DEFINE_RAND_FULL (randu,    unsigned,           unsigned)
DEFINE_RAND_RANGE(randu_r,  unsigned,           unsigned)
DEFINE_RAND_FULL (randl,    long,               unsigned long)
DEFINE_RAND_RANGE(randl_r,  long,               unsigned long)
DEFINE_RAND_FULL (randul,   unsigned long,      unsigned long)
DEFINE_RAND_RANGE(randul_r, unsigned long,      unsigned long)
DEFINE_RAND_FULL (randll,   long long,          unsigned long long)
DEFINE_RAND_RANGE(randll_r, long long,          unsigned long long)
DEFINE_RAND_FULL (randull,  unsigned long long, unsigned long long)
DEFINE_RAND_RANGE(randull_r,unsigned long long, unsigned long long)

/* =================== floating point =================== */

double randd(void) {
    /* 53-bit mantissa from the top bits of a u64 sample */
    return (next_u64() >> 11) * (1.0 / 9007199254740992.0);
}

double randd_r(double min, double max) {
    if (min == max) return min;
    if (min > max) { double t = min; min = max; max = t; }
    return min + (max - min) * randd();
}

float randf(void) {
    return (float)((next_u64() >> 40) * (1.0 / 16777216.0));
}

float randf_r(float min, float max) {
    if (min == max) return min;
    if (min > max) { float t = min; min = max; max = t; }
    return min + (max - min) * randf();
}

/* =================== bool / normal / compat =================== */

int randb(void) { return (int)(next_u64() >> 63); }

double randn(void) {
    /* Stateless, loop-free Box-Muller. */
    double u1 = randd();
    double u2 = randd();
    if (u1 == 0.0) u1 = 0x1p-53;   /* avoid log(0); 1-in-2^53 event */
    return sqrt(-2.0 * log(u1)) *
           cos(6.283185307179586476925286766559 * u2);
}

