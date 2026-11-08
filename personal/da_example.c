#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>    // strcasecmp
#include <sys/types.h>  // ssize_t
#include "da.h"

typedef struct {
    char **items;
    size_t count;
    size_t capacity;
} Strings;

int main(void)
{
    Strings lines = {0};

    // getline()'s buffer: it allocates it for us, and may realloc() it on
    // each call. We own it and must free() it exactly once.
    char  *buf     = NULL;
    size_t buf_cap = 0;

    puts("Enter lines. Type 'quit' (any case) or send EOF (Ctrl-D) to finish.");

    for (;;) {
        fputs("> ", stdout);
        fflush(stdout);

        ssize_t n = getline(&buf, &buf_cap, stdin);
        if (n < 0) break;                      // EOF or read error

        // Strip trailing newline / CR.
        while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r'))
            buf[--n] = '\0';

        if (n == 0) continue;                  // skip empty input
        if (strcasecmp(buf, "quit") == 0) break;

        // *** OWNERSHIP DECISION ***
        // `buf` will be reused (and possibly realloc'd) on the next
        // getline() call, so we must NOT store the pointer. Copy it.
        char *copy = strdup(buf);              // we now own `copy`
        if (!copy) { perror("strdup"); break; }

        da_append(&lines, copy);
    }

    // We're done with getline's buffer.
    free(buf);
    buf     = NULL;
    buf_cap = 0;

    // ---- use the data ----
    printf("\nYou entered %zu line(s):\n", lines.count);
    for (size_t i = 0; i < lines.count; ++i)
        printf("  [%zu] \"%s\"\n", i + 1, lines.items[i]);

    // ---- teardown ----
    // 1) free each string we own
    for (size_t i = 0; i < lines.count; ++i)
        free(lines.items[i]);
    // 2) free the backing array
    da_free(&lines);

    return 0;
}
