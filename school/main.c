#include "my_utils.h"
#include "my_io.h"
#include "arr_op.h"

void fn_1(void);
void fn_2(void);
void fn_3(void);
void fn_4(void);
void fn_5(void);
void fn_6(void);

bool is_number(const char *txt, int *num) {
    return strlen(txt) == (size_t)parse_i(txt, num);
}
int main(int argc, char **argv) {
    char *arg = *shift_args(&argc, &argv);
    if (argc == 0) {
        printf("Usage : %s <feladat>\n", arg);
        return 1;
    }

    arg = *shift_args(&argc, &argv);
    int num = 0;
    if (!is_number(arg, &num)) {
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
        default:
            err("not a valid num");
            break;
    }
#if defined(MEMCHECK)
    check_ptr_arr();
#endif

    return 0;

}

void fn_1(void) {
    char inp[21];
    if (!read_str("Input a line: ", ARR_L(inp))) {
        err("input failed");
        return;
    }
    size_t len = strlen(inp);
    char *cpy = New(char, len + 1); // auto cast
    if (!cpy) { // could extract the check to a macro too !!
        perror("copy of string");
        return;
    }
    memcpy(cpy, inp, len + 1); // copy the nul term. too
    printf("copy is: \"%s\"\n", cpy);
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
        size_t len = strlen(inp) + 1; // copy the nul term. too
        char *tmp = Re_New(char, arr, size + len);
        if (!tmp) {
            perror("copy of string");
            goto dealloc;
        }
        arr = tmp;
        memcpy(arr + size, inp, len);
        size += len;
    }
    char *end = arr+size-1;
    for(int i = 5; i >0; i--) {
        // forditott sorrend, forditva
        end--;
        for (;*end != '\0'; end--)
            fputc(*end, stdout);
        fprintf(stdout, "\n"); // this also fflush() -es it
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

        if(!resize_buff_t((void**)&arr, &size, sizeof(int))) {
            perror("resize");
            goto dealloc;
        }
        arr[count++] = num;

    }
    arr_rev(arr, count);
    arr_print(arr, count);
    destroy(arr);
}

void arr_rev_str(char *arr, size_t len) {                                                    
    if (!arr || len < 2)
        return;

    for (size_t i = 0; i < len / 2u; i++) { 
        char ty = arr[i]; 
        arr[i] = arr[len - i - 1u]; 
        arr[len - i - 1u] = ty;
    }
}
void fn_4(void) {
    char tmp[100];
    if (!read_str("Input a string: ", ARR_L(tmp))) {
        err("input failed");
        return;
    }
    arr_rev_str(ARR_Str(tmp));

    printf("reversed : %s\n", tmp);
}

char *arr_rev_str_new(char *arr, size_t len) {                                                    
    char *nw = New(char, len + 1);
    if (!nw || !arr || !len) 
        return NULL;
    for (size_t i = 0; i < len; i++)
        nw[len - i - 1] = arr[i];
    return nw;
}
void fn_5(void) {
    char tmp[100];
    if (!read_str("Input a string: ", ARR_L(tmp))) {
        err("input failed");
        return;
    }
    char *rev = arr_rev_str_new(ARR_Str(tmp));
    printf("reversed : %s\n", tmp);
    destroy(rev);
}

void fn_6(void) {
    char **arr = NULL;
    size_t size = 0;
    size_t count = 0;
    goto body;
dealloc:;;
    if (arr && count > 0) {
        for(size_t i = 0; i<count; i++) {
            free(arr[i]);
        }
    }
    free(arr);
    return;
body:;;
    while(1) {
        char tmp[101];
        if (!read_str("Input a str: ", ARR_L(tmp))) {
            err("input failed");
            goto dealloc;
        }
        int num;
        if (is_number(tmp, &num) && num == 0)
            break;

        if(!resize_buff_t((void**)&arr, &size, sizeof(char*))) {
            perror("resize");
            goto dealloc;
        }
        size_t len = strlen(tmp);
        char *cpy = New(char, len + 1); // auto cast
        if (!cpy) { // could extract the check to a macro too !!
            perror("copy of string");
            goto dealloc;
        }
        memcpy(cpy, tmp, len + 1); // copy the nul term. too
        arr[count++] = cpy;
    }
    arr_print_str(arr, count);
    destroy(arr);
}



