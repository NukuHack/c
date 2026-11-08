#ifndef RAND_H
#define RAND_H

#include <stdint.h>

void     seed_rng(uint64_t seed);

uint64_t randu64(void);                          /* raw 64-bit */

/* floating point */
double   randd(void);                            /* [0, 1)         */
double   randd_r(double min, double max);        /* [min, max)     */
float    randf(void);                            /* [0, 1)         */
float    randf_r(float min, float max);          /* [min, max)     */

/* bool */
int      randb(void);                            /* 0 or 1         */

/* integer family — same shape for every width */
int                randi(void);                  /* full int        */
int                randi_r(int min, int max);    /* [min, max]      */
unsigned           randu(void);
unsigned           randu_r(unsigned min, unsigned max);
long               randl(void);
long               randl_r(long min, long max);
unsigned long      randul(void);
unsigned long      randul_r(unsigned long min, unsigned long max);
long long          randll(void);
long long          randll_r(long long min, long long max);
unsigned long long randull(void);
unsigned long long randull_r(unsigned long long min, unsigned long long max);

/* normal */
double   randn(void);    /* (0, 1)         */

#endif /* RAND_H */
