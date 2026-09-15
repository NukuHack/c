#include <stdio.h>
#include "mylib.h"

int mult_add(int a, int b) {
    return a * b + a + b;
}

int main(void) {
    const char* s = "Hello from C!";
    printf("%s\n", s);

    printf("mult_add(2, 3) = %d\n", mult_add(2, 3));

    printf("add_numbers(5, 7) = %d\n", add_numbers(5, 7));
    printf("multiply_numbers(5, 7) = %d\n", multiply_numbers(5, 7));

    printf("string_length(\"%s\") = %zu\n", s, string_length(s));
    printf("string_length(\"\") = %zu\n", string_length(""));
    printf("string_length(NULL) = %zu\n", string_length(NULL));
    return 0;
}