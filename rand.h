
#ifndef RAND_H
#define RAND_H

#include <stdint.h>

void     seed_rng(uint64_t seed);

uint64_t randu64(void);                          /* raw 64-bit */
double   rand01(void);                           /* uniform [0, 1) */
double   randd(double min, double max);          /* uniform [min, max) */
int      randb(void);                            /* 0 or 1 */

int      randi(void);                            /* full int range */
int      randi_range(int min, int max);          /* uniform [min, max] inclusive */

long     randl(void);                            /* full long range */
long     randl_range(long min, long max);        /* uniform [min, max] inclusive */

double   randn(void);                            /* standard normal N(0,1) */

#endif /* RAND_H */

