#include "rand.h"

#include <math.h>
#include <stdint.h>

#define BIG_9E3  0x9E3779B97F4A7C15ULL
#define BIG_BF5  0xBF58476D1CE4E5B9ULL
#define BIG_940  0x94D049BB133111EBULL

static uint64_t rng_state[2] = { BIG_9E3, BIG_BF5 };

/* ---------- seeding ---------- */

static inline uint64_t splitmix64(uint64_t *x) {
    uint64_t z = (*x += BIG_9E3);
    z = (z ^ (z >> 30)) * BIG_BF5;
    z = (z ^ (z >> 27)) * BIG_940;
    return z ^ (z >> 31);
}

void seed_rng(uint64_t seed) {
    rng_state[0] = splitmix64(&seed);
    rng_state[1] = splitmix64(&seed);
    if (rng_state[0] == 0 && rng_state[1] == 0) rng_state[0] = 1;
}

/* ---------- core engine ---------- */

static inline uint64_t xorshift128plus(void) {
    uint64_t s1 = rng_state[0];
    const uint64_t s0 = rng_state[1];
    rng_state[0] = s0;
    s1 ^= s1 << 23;
    rng_state[1] = s1 ^ s0 ^ (s1 >> 17) ^ (s0 >> 26);
    return rng_state[1] + s0;
}

/* Unbiased uniform integer in [0, bound-1].  bound must be >= 1. */
static inline uint64_t bounded_u64(uint64_t bound) {
    /* number of values to reject so that 'r % bound' is uniform */
    uint64_t threshold = (uint64_t)(-bound) % bound;  /* = 2^64 % bound */
    uint64_t r;
    do { r = xorshift128plus(); } while (r < threshold);
    return r % bound;
}

/* ---------- raw / uniform ---------- */

uint64_t randu64(void) { return xorshift128plus(); }

double rand01(void) {
    /* 53-bit mantissa, top bits of a 64-bit sample */
    return (xorshift128plus() >> 11) * (1.0 / 9007199254740992.0); /* 2^-53 */
}

double randd(double min, double max) {
    return min + (max - min) * rand01();
}

int randb(void) {
    return (int)(xorshift128plus() >> 63);
}

/* ---------- int ---------- */

int randi(void) {
    /* full 32-bit int range, uniform */
    return (int)(int32_t)(xorshift128plus() >> 32);
}

int randi_range(int min, int max) {
    if (min > max) { int t = min; min = max; max = t; }
    uint64_t span = (uint64_t)((int64_t)max - (int64_t)min) + 1; /* <= 2^32 */
    return (int)((int64_t)min + (int64_t)bounded_u64(span));
}

/* ---------- long ---------- */

long randl(void) {
    /* works whether long is 32-bit or 64-bit */
    if (sizeof(long) == 4) return (long)(int32_t)(xorshift128plus() >> 32);
    return (long)(int64_t)xorshift128plus();
}

long randl_range(long min, long max) {
    if (min > max) { long t = min; min = max; max = t; }
    uint64_t span = (uint64_t)max - (uint64_t)min + 1;
    if (span == 0) {
        /* full uint64 span (e.g. LONG_MIN..LONG_MAX on 64-bit long) */
        return (long)xorshift128plus();
    }
    return (long)((uint64_t)min + bounded_u64(span));
}

/* ---------- normal ---------- */

double randn(void) {
    static int has_spare = 0;
    static double spare;

    if (has_spare) { has_spare = 0; return spare; }

    double u, v, s;
    do {
        u = 2.0 * rand01() - 1.0;
        v = 2.0 * rand01() - 1.0;
        s = u * u + v * v;
    } while (s >= 1.0 || s == 0.0);

    double mul = sqrt(-2.0 * log(s) / s);
    spare = v * mul;
    has_spare = 1;
    return u * mul;
}

