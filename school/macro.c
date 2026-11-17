#include "my_utils.h" // better io (renamed from small_lib.h)
#include "my_io.h" // input


// Basic string concat .. yet sooo powerful
#define CAT_I(a, b) a##b
#define CAT(a, b) CAT_I(a, b)

/* bool */
#define NOT(c) CAT(NOT_, c)
#define NOT_0 1
#define NOT_1 0

// this is insainly unsafe - what if the result is not 0 or 1 ...
#define IF_0(t, f) f
#define IF_1(t, f) t
#define IF(cond, t, f) CAT(IF_, cond)(t, f)
#define IF_T(cond, t) IF(cond, t, (void)0)

#define SWITCH(cond) CAT(SWITCH_, cond)
#define SWITCH_M(cond, x) CAT(CAT(SWITCH_, cond), x)

typedef char *char_ptr; // since preprocessor does not like "char*"
typedef char *str; // "define" does not work either, since that get "transformed" back
typedef const char *char_cptr;
typedef const char *cstr;

#define String_Fmt "%s"
#define Decimal_Fmt "%d"
#define Floating_Fmt "%f"

// since this is a macro, but just a concat, it is really easy to extend to anything new from other files
#define SWITCH_FMT_int Decimal_Fmt
#define SWITCH_FMT_long "%ld"
#define SWITCH_FMT_double Floating_Fmt
#define SWITCH_FMT_char "%c"
#define SWITCH_FMT_str String_Fmt
#define SWITCH_FMT_cstr String_Fmt
#define SWITCH_FMT_char_ptr String_Fmt
#define SWITCH_FMT_char_cptr String_Fmt
#define SWITCH_FMT_uint "%u"
#define SWITCH_FMT_float Floating_Fmt

#define SWITCH_GET_int inpi()
// TODO : long
#define SWITCH_GET_long (long)inpi()
#define SWITCH_GET_double inpd()
#define SWITCH_GET_char inpc()
#define SWITCH_GET_str inps()
#define SWITCH_GET_cstr inps()
#define SWITCH_GET_char_ptr inps()
#define SWITCH_GET_char_cptr inps()
#define SWITCH_GET_uint inpu()
// TODO : float
#define SWITCH_GET_float (float)inpd()

#define SWITCH_FMT(ty) SWITCH_M(FMT_, ty)
#define SWITCH_GET(ty) SWITCH_M(GET_, ty)

// "high level wrappers"
#define printF(ty, val) printf(SWITCH_FMT(ty) "\n", val)
#define VAR(ty, name, val) ty name = val;
#define GET(ty, name) VAR(ty, name, SWITCH_GET(ty))


#if ZERO

/*  *variadic* conditional  */
#define WHEN_1(...) __VA_ARGS__
#define WHEN_0(...)
#define WHEN(c) CAT(WHEN_, c)

// empty
#define EMPTY()
#define DEFER(id) id EMPTY()
// delays expansion by two rescans (needed because WHEN() rescans its body once more)
#define DEFER2(id) id EMPTY EMPTY() ()

// end check
#define COMMA ,
#define PROBE() ~, 1
#define CHECK_N(x, n, ...) n
#define CHECK(...) CHECK_N(__VA_ARGS__, 0)
#define IS_END_END PROBE()
#define IS_END(x) CHECK(IS_END_##x) // if we are at "END" then this expands to IS_END_END
                                    // otherwise we call check, with one "bad" input + more things

#define EVAL(...)  EVAL1(EVAL1(__VA_ARGS__))
#define EVAL1(...) EVAL2(EVAL2(__VA_ARGS__))
#define EVAL2(...) EVAL3(EVAL3(__VA_ARGS__))
#define EVAL3(...) EVAL4(EVAL4(__VA_ARGS__))
#define EVAL4(...) EVAL5(EVAL5(__VA_ARGS__))
#define EVAL5(...) __VA_ARGS__

/* Emit  SWITCH_FMT(ty)  for each pair, " " between.  sep = 0 for first, 1 after. */
#define WALK_FMT(...) EVAL(WALK_FMT_A(0, __VA_ARGS__, END, X))
#define WALK_FMT_A(sep, ty, val, ...) \
    WHEN(NOT(IS_END(ty)))(WHEN(sep)(" ") SWITCH_FMT(ty) DEFER2(WALK_FMT_B)(1, __VA_ARGS__))
#define WALK_FMT_B(sep, ...) DEFER2(WALK_FMT_A)(sep, __VA_ARGS__, END, X)

/* Emit  val  for each pair, "," between. */
#define WALK_VAL(...) EVAL(WALK_VAL_A(0, __VA_ARGS__, END, X))
#define WALK_VAL_A(sep, ty, val, ...) \
    WHEN(NOT(IS_END(ty)))(WHEN(sep)(COMMA) val DEFER2(WALK_VAL_B)(1, __VA_ARGS__))
#define WALK_VAL_B(sep, ...) DEFER2(WALK_VAL_A)(sep, __VA_ARGS__, END, X)

/* ---------- final macro ---------- */
#define printFF(...) \
    printf(WALK_FMT(__VA_ARGS__) "\n", WALK_VAL(__VA_ARGS__))

#endif

// disallowed : this is a c11 feature
#define printF_(val) _Generic((val), \
    int: printf("%d\n", (int)(val)), \
    long: printf("%ld\n", (long)(val)), \
    double: printf("%f\n", (double)(val)), \
    char: printf("%c\n", (char)(val)), \
    char*: printf("%s\n", (char*)(val)), \
    const char*: printf("%s\n", (const char*)(val)), \
    default: printf("%p\n", (void*)(val)) \
)

int main(int argc, char **argv) {
    if (argc == 1) {
        printf("Usage : %s <num>\n", argv[0]);
        return 1;
    }
    printF(str, argv[1]);

    VAR(int, score, 100);
    printF(int, score);

    GET(double, l_score);
    printF(double, l_score);

    VAR(cstr, name, "hello");
    printF(cstr, name);
    // printFF(cstr, name, int, score);
}

