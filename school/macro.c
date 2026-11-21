#include "my_utils.h" // better io (renamed from small_lib.h)
#include "my_io.h" // input

// empty
#define EMPTY()
#define DEFER(id) id EMPTY()
// delays expansion by two rescans (needed because WHEN() rescans its body once more)
#define DEFER2(id) id EMPTY EMPTY() ()

/*  *variadic* conditional  */
#define WHEN_1(...) __VA_ARGS__
#define WHEN_0(...)
#define WHEN(c) CAT(WHEN_, c)

#define SWITCH(cond) CAT(SWITCH_, cond)

// since this is a macro, but just a concat, it is really easy to extend to anything new from other files
//   could make it safer, with default fallback (here it could be just an error print at compiletime)
//   but that would need much more checking prop. __VA_ARGS__ so pain ...
#define SWITCH_FMT(ty) CAT(FMT_, x)
#define SWITCH_GET(ty) CAT(GET_, x)

// "high level wrappers"
#define printF(ty, val) printf(SWITCH_FMT(ty) "\n", val)
#define VAR(ty, name, val) ty name = val;
#define GET(ty, name) VAR(ty, name, SWITCH_GET(ty))


#define arr_print(ty, arr, len) \
    arr_foreach(ty, var, (arr), (len)) \
        printf("idx %zu) : "SWITCH_FMT(ty)"\n", arr_for_i, var);



// end check
#define COMMA ,
#define PROBE() ~, 1
#define CHECK_N(x, n, ...) n
#define CHECK(...) CHECK_N(__VA_ARGS__, 0, ~)
// if we are at "END" then this expands to IS_END_END
// otherwise we call check, with one "bad" input + more things
#define IS_END(x) CHECK(IS_END_##x)
#define IS_END_END PROBE()

/* -- the default branch -- */
#define CT_CASE_DEFAULT PROBE // never used, but just in case
#define CT_BODY_DEFAULT puts("default");

/* - cases: one marker + one body each - */
#define CT_CASE_1 PROBE
#define CT_BODY_1 puts("one");

#define CT_CASE_2 PROBE
#define CT_BODY_2 puts("two");

/* --- Switch dispatch --- */
#define CT_SWITCH(v) \
    IF( CHECK(CAT(CT_CASE_, v)()), \
        CAT(CT_BODY_, v), \
        CT_BODY_DEFAULT )

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



/*
    __STDC__: Must expand to the integer constant 1 in a conforming implementation.
    __STDC_VERSION__: Must expand to 199901L to indicate C99 conformance.
    __STDC_HOSTED__: Must be defined. Its value is 1 if the implementation is hosted, and 0 if it is freestanding.
    __DATE__ and __TIME__: Must expand to string literals representing the compilation date and time.
    __FILE__ and __LINE__: Must expand to the current source file name and line number.
 */





void _zero_or_die(int v, const char* filename, int line) {
    if (v != 0) {
       fprintf(stderr, "error %s:%d\n", filename, line);
       printERR;
       exit(1);
    }
}

#define ZERO_OR_DIE_ for (int _i=1; _i == 1; _zero_or_die(_i, __FILE__, __LINE__)) _i=



/*! \brief Compile-time assertion.
 *
 *  Note that the cassert() macro generates no code, and hence need not
 *  be restricted to debug builds.  It does have the side-effect of
 *  declaring a type name with typedef.  For this reason, a unique
 *  number or string of legal identifier characters must be included
 *  with each invocation to avoid the attempt to redeclare a type.
 *
 *  A failed assertion will attempt to define a type that is an array
 *  of -1 integers, which will throw an error in any standards
 *  compliant compiler. The exact error is implementation defined, but
 *  since the defined type name includes the string "ASSERTION" it
 *  should trigger curiosity enough to lead the user to the assertion
 *  itself.
 *
 *  Because a typedef is used, cassert() may be used inside a function,
 *  class or struct definition as well as at file scope.
 */
#define cassert(x,i) typedef int ASSERTION_##i[(x)?1:-1]

#define IMPLIES(x, y) (!(x) || (y))
#define COLUMNS(S,E) [ (E) - (S) + 1 ]

// no, just no
#define REQUIRE_ARRAY(T, X) (void)(sizeof (((char(*)(T(*)[]))0)(&(X))))
#define REQUIRE_ARRAY2(T, X) (void)(sizeof (((char(*)(T(*)[*][*]))0)(&(X))))


int macro_main(int argc, char **argv) {
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


    printF(cstr, __DATE__);
    printF(cstr, __TIME__);


    printFF(cstr, name, int, score);
    
    CT_SWITCH(1)   /* puts("one");    */
    CT_SWITCH(2)   /* puts("two");    */
    CT_SWITCH(42)  /* puts("default"); */

    return 0;
}

