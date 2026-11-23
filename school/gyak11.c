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

// Why use rust if we can have MACROs
//  the actual "zero cost abstraction" !!
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
        default:
            err("not a valid num");
            break;
    }

#if defined(MEMCHECK)
    check_mem_freed();
#endif
    return 0;
}

void init_arrs(int *id, double *avg, short *age, size_t len) {
    arr_foreach_p(int, var, id, len)
        *var = randi_b(100000, 0);
    arr_foreach_p(double, var, avg, len)
        *var = randd_b(5, 1);
    arr_foreach_p(short, var, age, len)
        *var = (short)randi_b(30, 15);
}
void fn_1(void) {
#define STUD_CNT 20
    int id[STUD_CNT];
    double avg[STUD_CNT];
    short age[STUD_CNT];
    init_arrs(id, avg, age, STUD_CNT);
    arr_print(int, id, STUD_CNT);
    arr_print(double, avg, STUD_CNT); // TODO : support for more precize formatting
    arr_print(short, age, STUD_CNT);
}

struct student {
    int id;
    short age;
    double avg;
}; // sizeof() = 16 at best
typedef struct student student_t; // student _ type
typedef student_t *student_p; // stident _ pointer

student_p student_init(void) {
    student_p st = New(student_t, 1);
    st->id = randi_b(10000, 0),
    st->age = (short)randi_b(30, 15),
    st->avg = randd_b(1, 5);
    return st;
}
#define ARG_student_t(st) (st).id, (st).age, (st).avg
#define FMT_student_t "id='"FMT_int"', age='"FMT_short"' avg='"FMT_double"'"
void student_print(student_p st) {
    printf("student: "FMT_student_t"\n", ARG_student_t(*st));
}
student_p student_search(student_p *students, size_t len) { // pointers > arrays
    if (!students || len == 0)
        return NULL;
    size_t idx = 0; double max = students[0]->avg;
    arr_foreach(student_p, st, students, len)
        if (st->avg > max) {
            idx = arr_for_i;
            max = st->avg;
        }
    return students[idx];
}
void fn_2(void) {
    printf("sizeof() of student : "FMT_size_t"\n", sizeof(student_t));
    student_p st = student_init();
    student_print(st);
    destroy(st);

    student_p *arr = NULL;
    size_t count = 0;
    size_t size = 0;
    forx(10) {
        IF_NGROW(student_p, arr, &size, count) {
            perror("student arr grow"); 
            destroy_arr(arr, count);
            return;
        }
        arr[count++] = student_init();
    }
    forx(10) {
        student_print(arr[i]);
    }
    student_p best = student_search(arr, count);
    if (best) {
        print("best student is:");
        student_print(best);
    } else {
        err("something bad");
    }

    destroy_arr(arr, count);
}

void fn_3(void) {
    err("TODO");
}


