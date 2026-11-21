#include "my_utils.h"
#include "my_io.h"
#include "arr_op.h"

void fn_1(void);
void fn_2(void);
void fn_3(void);
void fn_4(void);
void fn_5(void);
void fn_6(void);
void fn_7(void);

bool is_number(const char *txt, int *num) {
    size_t len = strlen(txt);
    return len != 0 && len == (size_t)parse_i(txt, num);
}
int main(int argc, char **argv) {
    char **arg = shift_args(&argc, &argv);
    if (argc == 0) {
        if (arg) {
            printf("Usage : %s <num>\n", *arg);
            return 1;
        } else {
            print("Usage : <executable> <num>");
            print(" - please exit from this bad environment please");
            return 1;
        }
    }

    arg = shift_args(&argc, &argv);
    int num = 0;
    if (!is_number(*arg, &num)) {
        perror("input number"); // only partially parsed
        return 1;
    }

    switch(num) {
        case(1):
            fn_1();
            break;
        case(2):
            fn_2();
            break;
        case(3):
            fn_3();
            break;
        case(4):
            fn_4();
            break;
        case(5):
            fn_5();
            break;
        case(6):
            fn_6();
            break;
        case(7):
            fn_7();
            break;
        default:
            err("not a valid num");
            break;
    }

#if defined(MEMCHECK)
    check_mem_freed();
#endif
    return 0;

}

void fn_1(void) {
    char inp[21];
    if (!read_str("Input a line: ", ARR_L(inp))) {
        err("input failed");
        return;
    }
    size_t len = strlen(inp) + 1;
    char *cpy = New(char, len); // auto cast
    if (!cpy) { // could extract the check to a macro too !!
        perror("copy of string");
        return;
    }
    memcpy(cpy, inp, len);
    printf("copy is: \"%s\"\n", cpy);
    destroy(cpy);
}

void fn_2(void) {
    // nem igazan ertem ezt a feladatot, szoval lesz igy
    char inp[100];
    char *arr = NULL;
    size_t size = 0;
    goto body;
dealloc:;;
    destroy(arr);
    return;
body:;;
    for (int i = 0; i<5; i++) {
        if (!read_str("Input a word: ", ARR_L(inp))) {
            err("input failed");
            goto dealloc;
        }
        size_t len = strlen(inp) + 1; // space for the nul term. too
        char *tmp = Re_New(char, arr, size + len);
        if (!tmp) {
            perror("copy of string");
            goto dealloc;
        }
        arr = tmp;
        memcpy(arr + size, inp, len);
        size += len;
    }
    char *cursor = arr + size;
    for (int i = 0; i < 5; i++) {
        if (cursor > arr)
            cursor--;

        while (cursor > arr && *(cursor - 1) != '\0')
            cursor--;

        print(cursor);
    }
    destroy(arr);
}

void fn_3(void) {
    int *arr = NULL;
    size_t size = 0;
    size_t count = 0;
    goto body;
dealloc:;;
    destroy(arr);
    return;
body:;;
    while(1) {
        char tmp[100];
        if (!read_str("Input a num: ", ARR_L(tmp))) {
            err("input failed");
            goto dealloc;
        }
        int num;
        if (!is_number(tmp, &num))
            break;

        IF_GROW(arr, &size, count, int) {
            perror("resize");
            goto dealloc;
        }
        arr[count++] = num;

    }
    arr_rev(arr, count);
    arr_print(int, arr, count);
    destroy(arr);
}

void fn_4(void) {
    char tmp[100];
    if (!read_str("Input a string: ", ARR_L(tmp))) {
        err("input failed");
        return;
    }
    arr_reverse(char, tmp, strlen(tmp));

    printf(" The reversed : %s\n", tmp);
}

void fn_5(void) {
    char tmp[100];
    if (!read_str("Input a string: ", ARR_L(tmp))) {
        err("input failed");
        return;
    }
    size_t len = strlen(tmp);
    arr_reverse(char, tmp, len);
    char *cpy = New(char, len + 1);
    if (!cpy) {
        perror("copy of string");
        return;
    }
    memcpy(cpy, tmp, len + 1);
    printf(" The reversed : %s\n", cpy);
    destroy(cpy);
}

void fn_6(void) {
    char **arr = NULL;
    size_t size = 0;
    size_t count = 0;
    goto body;
dealloc:;;
    destroy_arr(arr, count);
    return;
body:;;
    while(1) {
        char tmp[100]; // should change to "inps_s()"
        if (!read_str("Input a str: ", ARR_L(tmp))) {
            err("input failed");
            goto dealloc;
        }
        int num;
        if (is_number(tmp, &num) && num == 0)
            break;

        IF_GROW(arr, &size, count, char*) {
            perror("resize");
            goto dealloc;
        }
        size_t len = strlen(tmp);
        char *cpy = New(char, len + 1);
        if (!cpy) {
            perror("copy of string");
            goto dealloc;
        }
        memcpy(cpy, tmp, len + 1);
        arr[count++] = cpy;
    }
    arr_print(str, arr, count);
    goto dealloc;
}

void fn_7(void) {
    char tmp[100];
    if (!read_str("Input a string: ", ARR_L(tmp))) {
        err("input failed");
        return;
    }
    int c = count_words(tmp, " ");
    printf("word count : %d\n", c);
}


