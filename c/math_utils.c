#include <stdio.h>

// This is a private helper function.
// It will NOT be exported because it's missing the '@public' tag.
/* Adds two integers. */
int internal_add(int a, int b) {
    return a + b;
}

/**
 * @public
 * Calculates the sum of two integers.
 * This is a very useful function for addition.
 * @param a The first integer.
 * @param b The second integer.
 * @return The sum of a and b.
 */
int add(int a, int b) {
    // We can call the private helper here.
    return internal_add(a, b);
}

/**
 * @public
 * Multiplies two floating-point numbers.
 * @param x The first number.
 * @param y The second number.
 * @return The product of x and y.
 */
float multiply(float x, float y) {
    return x * y;
}