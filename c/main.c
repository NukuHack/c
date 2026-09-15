// c/main.c
#include <stdio.h>
// We will generate this file
// resolved via -Iinclude 
#include "math_utils.h"

/**
 * @public
 * The main program entry point.
 * @param any args
 * @return code
 */
int main() {
    int sum = add(5, 3);
    float product = multiply(2.5f, 4.0f);

    printf("Sum: %d\n", sum);
    printf("Product: %f\n", product);

    // Now, hover over 'add' or 'multiply' below.
    // The documentation from math_utils.c should appear!
    int another_sum = add(10, 20);

    return 0;
}