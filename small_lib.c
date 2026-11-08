//self
#include "small_lib.h"
//impl
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <math.h>
#include "string.h"
#include "parse.h"

#define STR_BUF 100

int lib_main(void) {

    int size = inp_int_s("number: ", 0, 10);
    printf("got: %d\n", size);

    char* str = inp_str();
    print(str);
    free(str); str = NULL;

    char c = inp_char();
    printf("got char : %c\n", c);

    double d = inp_double();
    printf("double: %lf\n", d);

    print("10 random double:");
    for (int i = 0; i < 10; i++) {
        printf("%d), %lf\n", i, rand_double());
    }

    return 0;
}

int rand_int(int min, int max) {
    if (min == max) // only possible
        return max;
    if (min > max) {
        int t = min;
        min = max;
        max = t;
    }
    long r = (long)rand() % (long)(max - min + 1);
    return (int) r + min;
}
double rand_double(void) {
    return (double)rand() / ((double)RAND_MAX + 1);
}

void print(const char* str) {
    if (str) fputs(str, stdout);
    fputs("\n", stdout);
}
void clear(void) {
    fputs("\033[H\033[J", stdout);
    fflush(stdout);
}
void err(const char* str) {
    if (str) fputs(str, stderr);
    fputs("\n", stderr);
}

int cmp(int a, int b) {
    // a > 1
    // b > -1
    // a==b 0
    return (a > b) - (a < b);
}

void finp(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int inp_int(void) {
    return inp_int_s("input a num: ", 0, 2000);
}
/* // old one, using builtin stuff, does not hadle '\n' and handles EOF badly
int inp_int_s(const char* text, int min, int max) {
    int num, r;
    if (min > max) {
        int t = min;
        min = max;
        max = t;
    }
read:
    if (text) {
        fputs(text, stdout);
        fflush(stdout);
    }
    r = scanf("%d", &num);
    if (r == EOF) {
        print("input a number (int)");
        goto read;
    }
    finp();
    if (!r) {
        print("input a number (int)");
    } else if (num < min) {
        printf("add a bigger number, min : %d\n", min);
    } else if (num > max) {
        printf("add a smaller number, max : %d\n", max);
    } else {
        return num;
    }
    goto read;
}*/

int inp_int_s(const char* text, int min, int max) {
    while (true) {
        char* str = inp_str_s(text);
        if (!str) return 0;
        int inp;
        int res = parse_int_b(str, &inp, min, max);
        int len = str_len(str);
        free(str); str = NULL;

        if (res > 0 && res == len) return inp;

        switch (res) {
            case NOP : print(len ? "input a number": "input smething"); break;
            case OOB : printf("input a number in [%d,%d]\n", min, max); break;
            case OVER : print("number Waay to big"); break;
            default : print("one single simple number (int)");
        }
    }
}

double inp_double(void) {
    return inp_double_s("input a double: ", -INFINITY, INFINITY);
}
/* // Old version - bad like the rest
double inp_double_s(const char* text) {
    double num;
    int r;
read:
    if (text) {
        fputs(text, stdout);
        fflush(stdout);
    }

    r = scanf("%lf", &num);
    if (r == EOF) {
        print("input a number (double)");
        goto read;
    }
    finp();
    if (!r) {
        print("input a real number");
    } else {
        return num;
    }
    goto read;
}
*/

double inp_double_s(const char* text, double min, double max) {
    while (true) {
        char* str = inp_str_s(text);
        if (!str) return 0;
        double inp;
        int res = parse_double_b(str, &inp, min, max);
        int len = str_len(str);
        free(str); str = NULL;

        if (res > 0 && res == len) return inp;

        switch (res) {
            case NOP : print(len ? "input a number": "input smething"); break;
            case OOB : printf("input a number in [%f,%f]\n", min, max); break;
            case OVER : print("number Waay to big"); break;
            default : print("one single simple number (duble)");
        }
    }
}

// might add uint version later, but for now basic int with min-max should be enough

char inp_char(void) {
    return inp_char_s("input a char: ");
}
char inp_char_s(const char* text) {
    int ch, c, r;
read:
    if (text) {
        fputs(text, stdout);
        fflush(stdout);
    }

    ch = getchar();
    if (ch == EOF || ch == '\n') {
        print("input a char (char)");
        goto read;
    }
    r = 1;
    while ((c = getchar()) != EOF && c != '\n') {
        r = 0;
    }
    if (!r) {
        print("input a char (char)");
    } else if (!is_alpha(ch)) {
        print("wrong char [a-zA-Z]");
    } else {
        return (char)ch;
    }
    goto read;
}

char* inp_str(void) {
    return inp_str_s("input a line: \n");
}

char* inp_str_s(const char *text) {
read:
    if (text) {
        fputs(text, stdout);
        fflush(stdout);
    }

    String* v = sv_create();
    int got_any = 0;
    int c;

    while ((c = getchar()) != EOF) {
        got_any = 1;

        if (c == '\n') {
            if (!sv_push(v, '\0'))
                goto free;

            return sv_free_get(v);
        }

        if (!sv_push(v, (char)c))
            goto free;
    }

    if (!got_any) {
        clearerr(stdin);
        print("\ntry again"); // prefix with new line, since we did not get anything
        // including the new line that should have been created by the Enter (by user)
        goto read;
    }

    /* EOF after a partial line: accept what we got */
    if (!sv_push(v, '\0')) {
        goto free;
    }
    return sv_free_get(v);
free:
    sv_free(v);
    return NULL;
}
/* // old broken, when i tried to add '\n' handling i gave up
   char* inp_str_s(const char* text) {
   char tmp[STR_BUF];
   char* str;
   int read, c;
read:
if (text) fputs(text, stdout);
c = getchar();

if (c == EOF) {
print("try again");
goto read;
} else if (c == '\n') {
goto memalloc; // we do allow empty or a single \n as string, caller should handle it
}

tmp[0] = (char)c;
read = 1;

while ((c = getchar()) != '\n' && c != EOF && read < STR_BUF) {
tmp[read++] = (char)c;
}
if (read == STR_BUF)
finp();
memalloc:
str = (char*)malloc(sizeof(char) * (read + 1));
if (!str) {
err("malloc failed");
return NULL;
}
for (int i = 0; i < read; i++)
str[i] = tmp[i];
str[read] = '\0';
return str;
}
*/

