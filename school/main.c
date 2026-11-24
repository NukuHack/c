#include "my_utils.h"
#include "my_io.h"
#include "arr_op.h"
#include "my_vec.h"

void fn_1(void);
void fn_2(void);
void fn_3(void);

cstr file_name = NULL;
int main(int argc, char **argv) {
    char **arg = shift_args(&argc, &argv);
    if (argc < 2) {
        if (arg) {
            printf("Usage : %s <num> <file>\n", *arg);
            return 1;
        } else {
            err("- please exit from this bad environment please");
            return 1;
        }
    }

    arg = shift_args(&argc, &argv);
    int num = 0;
    if (!is_number(*arg, &num)) {
        perror("input number");
        return 1;
    }

    file_name = *shift_args(&argc, &argv);

    switch(num) {
        case(1):
            fn_1();
            break;
        case(2):
            fn_2();
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

typedef struct {
    size_t capacity;
    size_t count;
    char *data;
} String;

void trim_l(str line, String *out) {
    *out = (String){0};
    if (!line) return;
    while (*line && isspace((unsigned char)*line)) line++;
    size_t len = strlen(line);
    while (len > 0 && isspace((unsigned char)line[len - 1])) len--;
    vec_alloc(*out, len);
    vec_push_arr(*out, line, len);
}
void fn_1(void) {
    bool f = check_file_exist(file_name);
    if (!f) {
        perror("file check");
        return;
    }
    FILE *file = fopen(file_name, "r");
    if (!file) {
        print(file_name);
        perror("file open");
        return;
    }

    str file_data = NULL;
    size_t len = 0;
    ptrdiff_t red = fgetbatch(&file_data, &len, file, false);
    if (red < 0) {
        perror("file read");
        free(file_data);
        return;
    } else if (red == 0) {
        print("file empty");
        return;
    }

    char* line = strtok(file_data, "\n");
    while (line) {
        String s; trim_l(line, &s);
        vec_print(s, char);
        vec_destroy(s);
        line = strtok(NULL, "\n");
    }

    destroy(file_data);
}

void fn_2(void) {
}

void fn_3(void) {
}


