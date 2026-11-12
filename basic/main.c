// c/main.c
#include <stdio.h>
#include <string.h>
// We will generate this file
// resolved via -Iinclude 
#include "math_utils.h"


/**
 * @public
 * Basic input method
 * @param storage storage, the buffer to store the input string
 * @param prompt prompt, the message to display before reading input
 * @return void
 */
void read_in(char* storage, char* prompt) {
    printf("%s", prompt);
    fgets(storage, sizeof(storage), stdin);
    storage[strlen(storage) - 1] = '\0'; // end string nicely
}

/**
 * @public
 * The main program entry point.
 * @param any args
 * @return code
 */
int main() {
    int sum = add(5, 3);
    float product = multiply(2.5f, 4.0f);

    char name1[100] = "";
    char name2[100] = "";

    printf("Sum: %d\n", sum);
    printf("Product: %f\n", product);

    // Now, hover over 'add' or 'multiply' below.
    // The documentation from math_utils.c should appear!
    int another_sum = add(10, 20);
    printf("Another sum: %d\n", another_sum);

    read_in(name1, "Enter first name: ");
    read_in(name2, "Enter second name: ");

    printf("Hello, %s %s!\n", name1, name2);


    return 0;
}