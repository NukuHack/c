/* lib.c -- the part that gets swapped at runtime. */

#ifdef _WIN32
#  define EXPORT __declspec(dllexport)   /* needed so the .dll exports the symbol */
#else
#  define EXPORT
#endif

#include <stdio.h>

EXPORT void say_hello(void)
{
    printf("Hello bread\n");
    fflush(stdout);   /* flush is important -- the .dll can be unloaded at any time */
}
