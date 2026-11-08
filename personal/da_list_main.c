#include "da.h"
#include <stdio.h>

typedef struct { int  *items; size_t count; size_t capacity; } IntArr;
typedef struct { int **items; size_t count; size_t capacity; } IntPtrArr;

static int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

static void section(const char *s) {
    printf("\n--- %s ---\n", s);
}

/* ===========================================================================
 * Auto-tour: every operation da.h provides.
 * =========================================================================*/
static void demo_ops(void) {
    section("append / append_many / print");
    IntArr xs = {0};
    da_init(&xs);                       // zero the fields (no alloc yet)

    da_append(&xs, 3);
    da_append(&xs, 1);
    da_append(&xs, 4);
    da_append(&xs, 1);
    da_append(&xs, 5);
    da_print(&xs, "%d");

    int more[] = { 9, 2, 6 };
    da_append_many(&xs, more, 3);
    da_print(&xs, "%d");

    section("sort");
    da_sort(&xs, cmp_int);
    da_print(&xs, "%d");

    section("insert / remove / remove_unordered");
    da_insert(&xs, 0, 100);             // prepend 100
    da_remove(&xs, 3);                  // shift-preserving delete
    da_remove_unordered(&xs, 1);        // O(1) delete (order not kept)
    da_print(&xs, "%d");

    section("pop / get / first / last / rep");
    int *p = da_pop(&xs);
    if (p) printf("popped %d\n", *p);

    int *g = da_get(&xs, 0);
    if (g) printf("xs[0] = %d\n", *g);

    printf("out-of-range get: %p\n", (void *)da_get(&xs, 999));
    printf("first = %d  last = %d\n", *da_first(&xs), *da_last(&xs));

    int old = xs.items[0];
    da_rep(&xs, 0, 999);
    printf("rep: %d -> %d\n", old, xs.items[0]);

    section("foreach (mutates in place, exposes index)");
    da_foreach(int, it, &xs) {
        printf("  [%td] = %d\n", it - xs.items, *it);
        *it += 100;
    }
    da_print(&xs, "%d");

    section("resize (zero-fill on growth) / truncate");
    da_resize(&xs, 10);
    da_print(&xs, "%d");
    da_truncate(&xs, 4);
    da_print(&xs, "%d");

    section("clear  (keeps capacity)");
    da_clear(&xs);
    printf("count=%zu capacity=%zu\n", xs.count, xs.capacity);
    da_free(&xs);

    /* ---- heap-allocated container -------------------------------------- */
#if defined(__GNUC__) || defined(__clang__)
    section("da_make / da_free_get");
    IntArr *hp = da_make(IntArr);       // malloc
    if (!hp) { puts("OOM"); return; }
    for (int i = 0; i < 5; i++) da_append(hp, i * i);
    da_print(hp, "%d");

    int *raw = (int *)da_free_get(hp);  // free struct, hand back items[]
    printf("raw[2] = %d\n", raw[2]);
    free(raw);                          // caller owns raw now
#endif

    /* ---- pointer elements + per-element malloc ------------------------- */
    section("da_resize_byte  (T must be a pointer type)");
    IntPtrArr ptrs = {0};
    da_resize_byte(&ptrs, 3, sizeof(int));  // 3 slots, each `malloc(sizeof(int))`
    *ptrs.items[0] = 10;
    *ptrs.items[1] = 20;
    *ptrs.items[2] = 30;

    da_foreach(int *, it, &ptrs)
        printf("[%td] %p = %d\n", it - ptrs.items, (void *)*it, **it);

    da_clear_fn(&ptrs, free);           // frees each element
    da_free(&ptrs);                     // frees the pointer array

    /* ---- shrink capacity ---------------------------------------------- */
    section("da_resize_auto");
    IntArr big = {0};
    da_reserve(&big, 1024);
    for (int i = 0; i < 10; i++) da_append(&big, i);
    printf("before: count=%zu capacity=%zu\n", big.count, big.capacity);
    da_resize_auto(&big);
    printf("after:  count=%zu capacity=%zu\n", big.count, big.capacity);
    da_free(&big);
}

/* ===========================================================================
 * Interactive demo 
 * =========================================================================*/
static void demo_input(void) {
    section("interactive (mirrors list_main)");

    int n;
    printf("how many ints (0 skips): ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        puts("skipped");
        return;
    }

    IntArr xs = {0};
    for (int i = 0; i < n; i++) {
        int v;
        if (scanf("%d", &v) != 1) break;
        da_append(&xs, v);
    }

    da_sort(&xs, cmp_int);
    da_print(&xs, "%d");
    da_free(&xs);
}

int main(void) {
    demo_ops();
    demo_input();
    return 0;
}
