/* =========================================================================
 * tiny_math.c  --  educational math library: powers, logs, trig, rounding.
 *
 * Core idea:  a^b = e^(b*ln a)  for a>0,  so we only need exp() and ln().
 * Everything else is built from a handful of primitives plus a Taylor series
 * whose convergence is guaranteed by range reduction.
 *
 * Int vs double, the "optimization" story in one sentence:
 *   ints get exact algebraic paths (repeated squaring, binary-search isqrt)
 *   or, for trig, are converted to double or handled by a lookup table;
 *   doubles get range-reduced Taylor series with ULP-level accuracy.
 *
 * [PROD] marks every place a production library would do more work
 *        (overflow checks, exact rounding, Payne-Hanek, frexp/ldexp, ...).
 * ========================================================================= */
#include <stdio.h>
#include <math.h>   /* NAN / INFINITY + cross-check in main() only */

/* ------------------------------------------------------------------ */
/* constants                                                          */
/* ------------------------------------------------------------------ */
#define EDU_PI        3.14159265358979323846
#define EDU_E         2.71828182845904523536
#define EDU_LN2       0.69314718055994530942
#define EDU_LN10      2.30258509299404568402
#define EDU_SQRT2     1.41421356237309504880
#define EDU_PHI       1.61803398874989484820
#define EDU_2_OVER_PI 0.63661977236758134308   /* 2/pi, for range reduction */

/* ------------------------------------------------------------------ */
/* tiny helpers                                                       */
/* ------------------------------------------------------------------ */

/* |x| for int, branch-free. */
static int edu_iabs(int x) {
    int mask = x >> 31;                 /* [PROD] INT_MIN is a corner case */
    return (x ^ mask) - mask;
}

/* |x| for double: clear the sign bit. */
static double edu_fabs(double x) {
    union { double d; unsigned long long u; } v;
    v.d = x;
    v.u &= 0x7FFFFFFFFFFFFFFFULL;
    return v.d;
}

/* x * 2^k.  [PROD] frexp/ldexp or exponent-bit manipulation is O(1). */
static double edu_ldexp(double x, int k) {
    if (k > 0) while (k--) x *= 2.0;
    else       while (k++) x *= 0.5;
    return x;
}

/* ------------------------------------------------------------------ */
/* INTEGER-ONLY PATHS  (exact, no FP)                                 */
/* ------------------------------------------------------------------ */

/* base^n, exponentiation by squaring. O(log n) multiplies. */
static int edu_ipow(int base, int n) {
    if (n < 0) return 0;                /* int negative power truncates */
    int r = 1;
    while (n) { if (n & 1) r *= base; base *= base; n >>= 1; }
    return r;
    /* [PROD] signed overflow is UB. Use unsigned or __builtin_mul_overflow. */
}

/* Exact floor(sqrt(x)) for unsigned. Binary search on the answer;
 * uses mid <= x/mid so mid*mid never overflows. */
static unsigned edu_isqrt(unsigned x) {
    unsigned lo = 0, hi = x, ans = 0;
    while (lo <= hi) {
        unsigned mid = lo + (hi - lo) / 2;
        if (mid == 0 || mid <= x / mid) { ans = mid; lo = mid + 1; }
        else                            { hi = mid - 1; }
    }
    return ans;
}

/* ------------------------------------------------------------------ */
/* DOUBLE PATHS: sqrt, exp, ln                                        */
/* ------------------------------------------------------------------ */

/* sqrt(x) by Newton-Raphson: g <- (g + x/g)/2.  Quadratic convergence. */
static double edu_sqrt(double x) {
    if (x <= 0.0) return 0.0;              /* [PROD] x<0 -> NaN */
    double g = (x >= 1.0) ? x : 1.0;
    for (int i = 0; i < 60; i++) {
        double ng = 0.5 * (g + x / g);
        if (ng == g) break;
        g = ng;
    }
    return g;
    /* [PROD] classic seed: g = bits-to-double(0x5f3759df - (bits(x)>>1))
       gets you to machine precision in ~2-3 iterations. */
}

/* e^x = 2^k * e^r  with  x = k*ln2 + r,  |r| <= ln2/2. */
static double edu_exp(double x) {
    if (x >  709.0) return INFINITY;       /* [PROD] check these! */
    if (x < -745.0) return 0.0;
    double kd = x / EDU_LN2;
    int k = (int)(kd >= 0.0 ? kd + 0.5 : kd - 0.5);
    double r = x - k * EDU_LN2;
    double sum = 1.0, term = 1.0;
    for (int n = 1; n <= 20; n++) { term *= r / n; sum += term; }
    return edu_ldexp(sum, k);
    /* [PROD] two-part ln2 (hi/lo) keeps k*ln2 exact for large |x|. */
}

/* ln(x) = k*ln2 + 2*atanh(t),  t = (m-1)/(m+1),  m in [1,2). */
static double edu_ln(double x) {
    if (x <= 0.0) return (x == 0.0) ? -INFINITY : NAN;   /* [PROD] */
    int k = 0;
    while (x >= 2.0) { x *= 0.5; k++; }    /* [PROD] frexp: O(1) */
    while (x <  1.0) { x *= 2.0; k--; }
    double t  = (x - 1.0) / (x + 1.0);     /* |t| <= 1/3 */
    double t2 = t * t;
    double term = t, sum = 0.0;
    for (int n = 1; n <= 41; n += 2) { sum += term / n; term *= t2; }
    return 2.0 * sum + k * EDU_LN2;
}

/* ------------------------------------------------------------------ */
/* POW                                                                */
/* ------------------------------------------------------------------ */

static double edu_pow_int(double base, int n) {
    int neg = (n < 0);
    unsigned e = neg ? -(unsigned)n : (unsigned)n;
    double r = 1.0;
    while (e) { if (e & 1) r *= base; base *= base; e >>= 1; }
    return neg ? 1.0 / r : r;
}

static double edu_pow(double a, double b) {
    if (a > 0.0) return edu_exp(b * edu_ln(a));
    if (a == 0.0) return (b > 0.0) ? 0.0 : INFINITY;
    long long n = (long long)b;                       /* [PROD] |b| < 2^63 */
    if ((double)n != b) return NAN;
    double p = edu_exp(b * edu_ln(-a));
    return (n % 2 != 0) ? -p : p;
}

static double edu_nth_root(double a, int n) {
    if (n <= 0) return NAN;
    if (n == 1) return a;
    if (a < 0.0) return (n % 2 == 0) ? NAN : -edu_nth_root(-a, n);
    if (a == 0.0) return 0.0;
    double x = a;
    for (int i = 0; i < 100; i++) {
        double nx = ((n - 1) * x + a / edu_pow_int(x, n - 1)) / n;
        if (nx == x) break;
        x = nx;
    }
    return x;
}

static double edu_pow_rat(double a, int m, int n) {
    return edu_nth_root(edu_pow_int(a, m), n);
}

/* ------------------------------------------------------------------ */
/* ROUNDING                                                           */
/*                                                                    */
/* For int, every "rounding" operation is the identity -- an int is   */
/* already integral.  So the interesting implementations are the ones */
/* for double, and the int "optimization" is simply: don't call them. */
/* ------------------------------------------------------------------ */

/* trunc = round toward zero (C cast semantics). */
static double edu_trunc(double x) {
    if (x >=  9223372036854775808.0) return x;   /* >= 2^63: out of LL range */
    if (x <  -9223372036854775808.0) return x;   /* < -2^63                  */
    if (x >= 0.0) return (double)(long long)x;
    return -(double)(long long)(-x);
}

/* floor = round toward -inf. */
static double edu_floor(double x) {
    double t = edu_trunc(x);
    return (t > x) ? t - 1.0 : t;                /* trunc overshot upward */
}

/* ceil = round toward +inf. */
static double edu_ceil(double x) {
    double t = edu_trunc(x);
    return (t < x) ? t + 1.0 : t;                /* trunc undershot downward */
}

/* round to nearest, ties away from zero (matches C's round()).
 * [PROD] x+0.5 can lose the .5 on huge x; rint()/nearbyint() and
 *        round-half-to-even are the well-defined alternatives. */
static double edu_round(double x) {
    return (x >= 0.0) ? edu_floor(x + 0.5) : edu_ceil(x - 0.5);
}

/* fmod(x,y) = x - trunc(x/y)*y.
 * [PROD] precision drops when x/y is huge; use repeated squaring or a
 *        correctly-rounded fmod. */
static double edu_fmod(double x, double y) {
    if (y == 0.0) return NAN;
    return x - edu_trunc(x / y) * y;
}

/* ------------------------------------------------------------------ */
/* LOGARITHMS                                                         */
/*                                                                    */
/* Note: in C, log() is the natural log and log10() is base 10.       */
/* Here we follow the mathematician's naming: edu_log10, edu_log2,    */
/* edu_ln, and a general edu_log(base, x).                            */
/* ------------------------------------------------------------------ */

static double edu_log10(double x) { return edu_ln(x) / EDU_LN10; }
static double edu_log2 (double x) { return edu_ln(x) / EDU_LN2;  }
static double edu_log  (double base, double x) {
    if (base <= 0.0 || base == 1.0) return NAN;      /* [PROD] */
    return edu_ln(x) / edu_ln(base);
}
/* Why divide?  Because  log_b(x) = ln(x) / ln(b)  is a definition, not
 * an approximation.  The only error is the error in ln().               */

/* ------------------------------------------------------------------ */
/* TRIG                                                               */
/*                                                                    */
/* Both sin(x) and cos(x) come from ONE range reduction and TWO short  */
/* Taylor series on [-pi/4, pi/4].  This is how real libms do it.      */
/*                                                                    */
/* For int arguments: trig is not meaningful without a domain (degrees? */
/* radians? grads?).  The usual choice is a small lookup table, e.g.   */
/*   sin_deg[d] for d in 0..359 with linear interp,                    */
/*   or a fixed-point (Q15/Q31) CORDIC.  We stick to doubles here.     */
/* ------------------------------------------------------------------ */

/* Compute sin(x) and cos(x) together.  More accurate and faster than
 * computing them separately because the expensive range reduction is
 * shared.  This is the classic libm sin_cos primitive.                 */
static void edu_sin_cos(double x, double *ps, double *pc) {
    /* Reduce to r = x - k*(pi/2),  |r| <= pi/4,  k = round(x/(pi/2)). */
    double kd = edu_round(x * EDU_2_OVER_PI);        /* k, as double */
    long long k = (long long)kd;                     /* [PROD] huge x */
    double r = x - kd * (EDU_PI / 2.0);              /* [PROD] two-part π/2 */

    /* sin(r) = r - r^3/3! + r^5/5! - ...   (|r| <= 0.786 => 12 terms ample) */
    double s = r, ts = r, r2 = r * r;
    for (int i = 1; i <= 12; i++) {
        ts *= -r2 / ((double)(2*i) * (2*i + 1));
        s  += ts;
    }
    /* cos(r) = 1 - r^2/2! + r^4/4! - ... */
    double c = 1.0, tc = 1.0;
    for (int i = 1; i <= 12; i++) {
        tc *= -r2 / ((double)(2*i - 1) * (2*i));
        c  += tc;
    }

    /* Undo the k*(pi/2) shift by quadrant rotation. */
    switch (((k % 4) + 4) % 4) {
        case 0: *ps =  s; *pc =  c; break;
        case 1: *ps =  c; *pc = -s; break;
        case 2: *ps = -s; *pc = -c; break;
        default:*ps = -c; *pc =  s; break;
    }
}

static double edu_sin(double x) { double s, c; edu_sin_cos(x, &s, &c); return s; }
static double edu_cos(double x) { double s, c; edu_sin_cos(x, &s, &c); return c; }

static double edu_tan(double x) {
    double s, c; edu_sin_cos(x, &s, &c);
    if (c == 0.0) return (s >= 0.0) ? INFINITY : -INFINITY;  /* [PROD] */
    return s / c;
}

/* atan via double half-angle reduction + Taylor.
 * atan(x) = 2*atan(x / (1 + sqrt(1+x^2)))  reduces |x| to ~0.2,
 * where the series  x - x^3/3 + x^5/5 - ...  converges fast.        */
static double edu_atan(double x) {
    if (x < 0.0)  return -edu_atan(-x);
    if (x > 1.0)  return EDU_PI / 2.0 - edu_atan(1.0 / x);

    /* Two half-angle steps: atan(x) = 4 * atan(y),  |y| <= 0.2. */
    double y = x;
    y = y / (1.0 + edu_sqrt(1.0 + y * y));
    y = y / (1.0 + edu_sqrt(1.0 + y * y));

    double term = y, sum = y, y2 = y * y;
    for (int k = 1; k <= 25; k++) {
        term *= -y2;
        sum  += term / (2*k + 1);
    }
    return 4.0 * sum;
}

/* atan2(y, x) picks the correct quadrant from the signs of x and y. */
static double edu_atan2(double y, double x) {
    if (x > 0.0) return edu_atan(y / x);
    if (x < 0.0) return (y >= 0.0) ? edu_atan(y / x) + EDU_PI
                                   : edu_atan(y / x) - EDU_PI;
    /* x == 0 */
    if (y > 0.0) return  EDU_PI / 2.0;
    if (y < 0.0) return -EDU_PI / 2.0;
    return 0.0;                                     /* atan2(0,0) = 0 */
}

/* asin(x) = atan(x / sqrt(1 - x^2))  for |x| < 1. */
static double edu_asin(double x) {
    if (x < -1.0 || x > 1.0) return NAN;
    if (x ==  1.0) return  EDU_PI / 2.0;
    if (x == -1.0) return -EDU_PI / 2.0;
    return edu_atan(x / edu_sqrt(1.0 - x * x));
}

/* acos(x) = 2*atan(sqrt((1-x)/(1+x))).  Stable near x = ±1 where the
 * naive  pi/2 - asin(x)  loses precision by cancellation.            */
static double edu_acos(double x) {
    if (x < -1.0 || x > 1.0) return NAN;
    if (x ==  1.0) return 0.0;
    if (x == -1.0) return EDU_PI;
    return 2.0 * edu_atan(edu_sqrt((1.0 - x) / (1.0 + x)));
}

/* ================================================================== */
/* DEMO                                                               */
/* ================================================================== */
int main(void) {
    printf("=== constants ===\n");
    printf("EDU_PI = %.15f\n", EDU_PI);
    printf("EDU_E  = %.15f  (edu_exp(1) = %.15f)\n",
           EDU_E, edu_exp(1.0));

    printf("\n=== powers ===\n");
    printf("3^1.5        = %.15f   (edu_pow)\n", edu_pow(3.0, 1.5));
    printf("sqrt(3^3)    = %.15f   (algebraic)\n",
           edu_sqrt(edu_pow_int(3.0, 3)));
    printf("edu_pow_rat(3,3,2) = %.15f\n", edu_pow_rat(3.0, 3, 2));
    printf("5^pi         = %.15f   (libm: %.15f)\n",
           edu_pow(5.0, EDU_PI), pow(5.0, EDU_PI));
    printf("edu_ipow(3,10)= %d          (exact, int)\n", edu_ipow(3, 10));
    printf("edu_pow(3,10) = %.15f   (log/exp path)\n", edu_pow(3.0, 10.0));

    printf("\n=== logs ===\n");
    printf("edu_ln(e)          = %.15f\n", edu_ln(EDU_E));
    printf("edu_log10(1000)    = %.15f\n", edu_log10(1000.0));
    printf("edu_log2(8)        = %.15f\n", edu_log2(8.0));
    printf("edu_log(3, 81)     = %.15f   (base-3 log)\n", edu_log(3.0, 81.0));
    printf("edu_log10(2)       = %.15f   (libm: %.15f)\n",
           edu_log10(2.0), log10(2.0));

    printf("\n=== trig: sin, cos, tan ===\n");
    double angles[] = { 0.0, EDU_PI/6, EDU_PI/4, EDU_PI/3, EDU_PI/2, EDU_PI,
                        2*EDU_PI, -EDU_PI/6, 10.0 };
    printf("   x          sin(x)          cos(x)          tan(x)\n");
    for (int i = 0; i < 9; i++) {
        double x = angles[i];
        double s, c; edu_sin_cos(x, &s, &c);
        double t = (c == 0.0) ? NAN : s / c;
        printf("%10.6f  %14.10f  %14.10f  %14.10f\n", x, s, c, t);
    }
    printf("edu_sin(pi/6) = %.15f  (should be 0.5)\n",
           edu_sin(EDU_PI/6));
    printf("edu_sin(pi/2) = %.15f  (should be 1)\n", edu_sin(EDU_PI/2));
    printf("edu_cos(0)    = %.15f\n", edu_cos(0.0));

    printf("\n=== inverse trig ===\n");
    printf("edu_atan(1)      = %.15f   (= pi/4 = %.15f)\n",
           edu_atan(1.0), EDU_PI/4);
    printf("edu_atan2(1, 1)  = %.15f\n", edu_atan2(1.0, 1.0));
    printf("edu_atan2(1, -1) = %.15f   (= 3pi/4)\n", edu_atan2(1.0, -1.0));
    printf("edu_atan2(-1,-1) = %.15f   (= -3pi/4)\n", edu_atan2(-1.0, -1.0));
    printf("edu_asin(0.5)    = %.15f   (= pi/6 = %.15f)\n",
           edu_asin(0.5), EDU_PI/6);
    printf("edu_acos(0.5)    = %.15f   (= pi/3 = %.15f)\n",
           edu_acos(0.5), EDU_PI/3);
    printf("round-trip sin(asin(0.7)) = %.15f\n", edu_sin(edu_asin(0.7)));

    printf("\n=== rounding ===\n");
    double vals[] = { 2.7, -2.7, 2.3, -2.3, 2.5, -2.5, 2.4, -2.4,
                      2.9999999999, -2.9999999999 };
    printf("    x            trunc      floor      ceil       round\n");
    for (int i = 0; i < 10; i++) {
        double v = vals[i];
        printf("%14.9f  %9.3f  %9.3f  %9.3f  %9.3f\n",
               v, edu_trunc(v), edu_floor(v), edu_ceil(v), edu_round(v));
    }
    /* For int, all of these are the identity function.
     * That is the "integer optimization": don't call them at all. */
    printf("(for int, floor/ceil/round/trunc are all identity)\n");

    printf("\n=== misc ===\n");
    printf("edu_fmod(7.5, 2.0) = %.1f\n", edu_fmod(7.5, 2.0));
    printf("EDU_PHI = %.15f\n", EDU_PHI);
    printf("edu_iabs(-42) = %d, edu_fabs(-42.5) = %.1f\n",
           edu_iabs(-42), edu_fabs(-42.5));
    return 0;
}