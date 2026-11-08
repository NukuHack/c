//self
#include "small_lib.h"
//impl
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include "string.h"

#define STR_BUF 100

#define NPE -1 // short for NullPointer
#define OVER -2 // short for overflow
#define OOB -3 // short for OutOfBounds
#define NOP 0 // could not parse (0 chars consumed, might not be bad)

// <num>u -> uint !! Soooo nice
int parse_main(void) {
    
    char* inp1 = inp_str_s("input an int: ");
    char* inp2 = inp_str_s("input a uint: ");
    char* inp3 = inp_str_s("input a double: ");
    char* inp4 = inp_str_s("input 2 int separated by a single non digit char: \n");
   
    int numx;
    int numy = parse_int(inp4, &numx);
    // +1 is for delimiter
    parse_int(inp4 + numy + 1, &numy);
    uint num1; parse_uint(inp2, &num1);
    double num2; parse_double(inp3, &num2);
    int num3; parse_int(inp1, &num3);

    printf("int got %d\n", num3);
    printf("uint got %u\n", num1);
    printf("double got %lf\n", num2);
    printf("num %d, num %d\n", numx, numy);

    return 0;
}

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

    return parse_main();
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



int parse_int(const char* text, int* num) {
    if (!text || !num) return NPE;
    const char* p = text;
    int sign = 0; // 0 = positive, 1 = negative
    if (*p == '-') { sign = 1; p++; }
    else if (*p == '+') { p++; }
    if (*p < '0' || *p > '9') return NOP; // non numer first char

    uint acc = 0; // "accumulator" so bsically storage
    uint limit = (uint)INT_MAX + (uint)sign;
    do {
        uint digit = (uint)(*p - '0');
        if (acc > (limit - digit) / 10u) return OVER;
        acc = acc * 10u + digit;
        p++;
    } while (*p >= '0' && *p <= '9');

    *num = sign != 0 ?
        (acc == limit) ? INT_MIN : -(int)acc // int_min is 1 'bigger' than int_max, so in fact *-1 is lossy
        : (int)acc;
    return (int)(p - text);
}
int parse_int_b(const char* text, int* num, int min, int max) {
    if (!num)
        return NPE;
    if (max < min) {
        int tmp = min;
        min = max;
        max = tmp;
    }
    int tmp;
    int chars = parse_int(text, &tmp);
    if (chars < 0)
        return chars;
    else if (chars == 0)
        return NOP;
    else if (tmp > max || tmp < min)
        return OOB;
    *num = tmp;
    return chars;
}


int parse_double(const char* text, double* num) {
    if (!text || !num) return NPE;
    const char* p = text;
    int sign = 0;
    if (*p == '-') { sign = 1; p++; }
    else if (*p == '+') { p++; }
    if (*p < '0' || *p > '9') return NOP;

    // idk where would a double overflow ... so the easiest way to solve a problem : ignore it
    // no branch returns OVERFLOW
    double acc = 0.0;
    while (*p >= '0' && *p <= '9') {
        acc = acc * 10.0 + (*p - '0');
        p++;
    }
    if (*p == '.') {
        p++;
        double scale = 0.1;
        while (*p >= '0' && *p <= '9') {
            acc += (*p - '0') * scale;
            scale *= 0.1;
            p++;
        }
    }

    *num = sign != 0 ? -acc : acc;
    return (int)(p - text);
}
int parse_double_b(const char* text, double* num, double min, double max) {
    if (!num)
        return NPE;
    if (max < min) {
        double tmp = min;
        min = max;
        max = tmp;
    }
    double tmp;
    int chars = parse_double(text, &tmp);
    if (chars < 0)
        return chars;
    else if (chars == 0)
        return NOP;
    else if (tmp > max || tmp < min)
        return OOB;
    *num = tmp;
    return chars;
}

int parse_uint(const char* text, uint* num) {
    if (!text || !num) return NPE;
    const char* p = text;
    // cant have -;+ since it is obviously a positive number ...
    if (*p < '0' || *p > '9') return NOP;

    uint acc = 0;
    do {
        uint digit = (uint)(*p - '0');
        if (acc > (UINT_MAX - digit) / 10u) return OVER;
        acc = acc * 10u + digit;
        p++;
    } while (*p >= '0' && *p <= '9');

    *num = acc;
    return (int)(p - text);
}
int parse_uint_b(const char* text, uint* num, uint min, uint max) {
    if (!num)
        return NPE;
    if (max < min) {
        uint tmp = min;
        min = max;
        max = tmp;
    }
    uint tmp;
    int chars = parse_uint(text, &tmp);
    if (chars < 0)
        return chars;
    else if (chars == 0)
        return NOP;
    else if (tmp > max || tmp < min)
        return OOB;
    *num = tmp;
    return chars;
}




