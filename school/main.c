#include "my_utils.h"
#include "my_io.h"
#include "arr_op.h"

void fn_1(void);
void fn_2(void);
void fn_3(void);

int main(int argc, char **argv) {
    char **arg = shift_args(&argc, &argv);
    if (argc < 1) {
        printf("Usage : %s <num>\n", *arg);
        return 1;
    }

    arg = shift_args(&argc, &argv);
    int num = 0;
    if (!try_parse_i(*arg, &num)) {
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
        default:
            puts("not a valid num");
            break;
    }

#if defined(MEMCHECK)
    check_mem_freed();
#endif
    return 0;
}

// now the random is Seeded correctly, so each run is actually random !
void init_arrs(int *id, double *avg, short *age, size_t len) {
    // arr_foreach -> just a basic foreach impl. for arrays (_p suffix means iterate through the pointers)
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
    // arr_print -> using type's format, in an arr_foreach
    arr_print(int, id, STUD_CNT);
    arr_print(double, avg, STUD_CNT); // TODO : support for more precize formatting, like decimal places
    arr_print(short, age, STUD_CNT);
}

struct student {
    int id;
    short age;
    double avg;
}; // sizeof() = 16 at best; 8 is the double, the rest is "nice" if can fit in another 8 bytes "__alignmentof()" or something
typedef struct student student_t; // student _ type
typedef student_t *student_p; // student _ pointer // had to make this so my macros work, mainly the foreach
student_p student_init(void) {
    student_p st = New(student_t, 1);
    st->id = randi_b(10000, 0),
    st->age = (short)randi_b(30, 15),
    st->avg = randd_b(1, 5);
    return st;
}
#define ARG_student_t(st) (st).id, (st).age, (st).avg /* TODO : make printF() accept ARG_<ty> "getters" */
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
    destroy(st); // kill the single student

    student_p *arr = NULL;
    size_t count = 0;
    size_t size = 0;
    forx(10) { // just a for(i=0;i<x;i++) simply
        // IF_NGROW : drow a dynamic array, if failed enter the body of the { }
        IF_NGROW(student_p, arr, &size, count) {
            perror("student arr grow"); 
            destroy_arr(arr, count);
            // del the entire array, count is the already insterted count, so should be correct
            return;
        }
        arr[count++] = student_init();
    }
    // arr_foreach -> just a basic foreach impl. for arrays (_p suffix means iterate through the pointers)
    arr_foreach(student_p, st, arr, count)
        student_print(st);

    student_p best = student_search(arr, count);
    if (best) {
        puts("best student is:");
        student_print(best);
    } else {
        puts("something bad");
    }

    destroy_arr(arr, count); // del the entire array
}

void fn_3(void) {
    puts("TODO");
}


