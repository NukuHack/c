// self
#include "parse.h"
// implementation
#include "small_lib.h"
#include <limits.h>
#include <stdio.h>

// <num>u -> unsigned !! Soooo nice
int parse_main(void) {
    
    char* inp1 = inp_str_s("input an int: ");
    char* inp2 = inp_str_s("input a uint: ");
    char* inp3 = inp_str_s("input a double: ");
    char* inp4 = inp_str_s("input 2 int separated by a single non digit char: \n");
   
    int numx;
    int numy = parse_int(inp4, &numx);
    // +1 is for delimiter
    parse_int(inp4 + numy + 1, &numy);
    unsigned num1; parse_uint(inp2, &num1);
    double num2; parse_double(inp3, &num2);
    int num3; parse_int(inp1, &num3);

    printf("int got %d\n", num3);
    printf("uint got %u\n", num1);
    printf("double got %lf\n", num2);
    printf("num %d, num %d\n", numx, numy);

    return 0;
}

int parse_int(char* text, int* num) {
    if (!text || !num) return NPE;
    char* p = text;
    int sign = 0; // 0 = positive, 1 = negative
    if (*p == '-') { sign = 1; p++; }
    else if (*p == '+') { p++; }
    if (*p < '0' || *p > '9') return NOP; // non numer first char

    unsigned acc = 0; // "accumulator" so bsically storage
    unsigned limit = (unsigned)INT_MAX + (unsigned)sign;
    do {
        unsigned digit = (unsigned)(*p - '0');
        if (acc > (limit - digit) / 10u) return OVER;
        acc = acc * 10u + digit;
        p++;
    } while (*p >= '0' && *p <= '9');

    *num = sign != 0 ?
        (acc == limit) ? INT_MIN : -(int)acc // int_min is 1 'bigger' than int_max, so in fact *-1 is lossy
        : (int)acc;
    return (int)(p - text);
}
int parse_int_b(char* text, int* num, int min, int max) {
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


int parse_double(char* text, double* num) {
    if (!text || !num) return NPE;
    char* p = text;
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
int parse_double_b(char* text, double* num, double min, double max) {
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

int parse_uint(char* text, uint* num) {
    if (!text || !num) return NPE;
    char* p = text;
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
int parse_uint_b(char* text, uint* num, uint min, uint max) {
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



