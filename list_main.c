/* main.c — tour of the List API from list.h */
#include "list.h"
#include <stdio.h>
#include <stdlib.h>

/* --------------------------------------------------------------------- */
static void section(const char *s) { printf("\n--- %s ---\n", s); }

/* List stores void*; for the demo we store heap int* so list_print works */
static int *mkint(int v) {
    int *p = (int *)malloc(sizeof *p);
    if (!p) { fprintf(stderr, "OOM\n"); exit(1); }
    *p = v;
    return p;
}

/* custom free_fn example (used by *_fn variants) */
static void my_free(void *p) {
    printf("  (custom free: %d)\n", p ? *(int *)p : -1);
    free(p);
}

/* ===================================================================== */
static void demo_ops(void) {
    section("make / put / print");
    List *list = list_make();                       // default capacity (DEF_SIZE)
    if (!list) { fprintf(stderr, "list_make failed\n"); return; }

    for (int i = 0; i < 5; i++) {
        int *v = mkint(i * i);
        if (!list_put(list, v)) { free(v); fprintf(stderr, "put failed\n"); }
    }
    list_print(list);
    printf("len=%zu sz=%zu\n", list_len(list), list->sz);

    section("get / head / tail / len");
    int *g = (int *)list_get(list, 0);
    printf("list[0]  = %d\n", g ? *g : -1);
    printf("get(999) = %p\n", (void *)list_get(list, 999));
    int *h = (int *)list_head(list);
    int *t = (int *)list_tail(list);
    printf("head=%d tail=%d len=%zu\n", h ? *h : -1, t ? *t : -1, list_len(list));

    section("ins");
    int *iv = mkint(999);
    if (list_ins(list, iv, 1)) list_print(list);
    else                       free(iv);

    section("rep");
    int *nv  = mkint(777);
    int *old = (int *)list_rep(list, nv, 0);
    printf("replaced %d with %d\n",
           old ? *old : -1, *(int *)list_get(list, 0));
    free(old);

    section("pop / rem");
    int *pv = (int *)list_pop(list);
    printf("popped   = %d\n", pv ? *pv : -1);
    free(pv);

    int *rv = (int *)list_rem(list, 0);
    printf("removed[0] = %d\n", rv ? *rv : -1);
    free(rv);
    list_print(list);

    section("reserve / grow");
    printf("before reserve: sz=%zu\n", list->sz);
    list_reserve(list, 128);
    printf("after  reserve(128): sz=%zu\n", list->sz);

    section("resize  (mallocs `byte` per new slot)");
    size_t before = list->len;
    if (list_resize(list, before + 5, sizeof(int))) {
        for (size_t i = before; i < list->len; i++)
            *(int *)list_get(list, i) = (int)(i * 10);
        list_print(list);
    }

    section("resize_auto  (shrink capacity)");
    printf("before: len=%zu sz=%zu\n", list->len, list->sz);
    if (list_resize_auto(list))
        printf("after:  len=%zu sz=%zu\n", list->len, list->sz);

    section("truncate  (shrink capacity, frees excess items)");
    printf("before: len=%zu sz=%zu\n", list->len, list->sz);
    if (list_truncate(list, 6)) {
        printf("after:  len=%zu sz=%zu\n", list->len, list->sz);
        list_print(list);
    }

    section("clear  (frees items + storage, keeps struct)");
    list_clear(list);
    printf("after clear: len=%zu sz=%zu arr=%p\n",
           list->len, list->sz, (void *)list->arr);

    section("re-init after clear");
    if (list_init_sz(list, 4)) {
        for (int i = 0; i < 3; i++) list_put(list, mkint(i + 100));
        list_print(list);
    }

    section("free  (items + storage + struct)");
    list_free(list);
    printf("freed\n");
}

/* ===================================================================== */
static void demo_make_sz(void) {
    section("make_sz(8)");
    List *list = list_make_sz(8);
    if (!list) return;

    for (int i = 0; i < 3; i++) list_put(list, mkint(i));
    list_print(list);
    list_free(list);
}

/* ===================================================================== */
static void demo_clear_fn(void) {
    section("clear_fn  (custom free_fn)");
    List *list = list_make();
    for (int i = 0; i < 4; i++) list_put(list, mkint(i));
    list_clear_fn(list, my_free);   /* my_free is called once per item */
    printf("after clear_fn: len=%zu sz=%zu\n", list->len, list->sz);
    list_free(list);
}

/* ===================================================================== */
static void demo_nuke(void) {
    section("nuke  (frees contents, KEEPS struct)");
    List *list = list_make();
    for (int i = 0; i < 3; i++) list_put(list, mkint(i));
    list_nuke(list);
    printf("after nuke: len=%zu sz=%zu arr=%p\n",
           list->len, list->sz, (void *)list->arr);

    /* struct is still usable → can re-init */
    list_init_sz(list, 4);
    list_put(list, mkint(42));
    list_print(list);
    list_free(list);
}

/* ===================================================================== */
static void demo_free_get(void) {
    section("free_get  (struct freed, caller owns items[])");
    List *list = list_make();
    for (int i = 0; i < 5; i++) list_put(list, mkint(i + 1));

    size_t len = list_len(list);
    void **arr = list_free_get(list);   /* list is now gone; arr ours */
    for (size_t i = 0; i < len; i++) {
        printf("raw[%zu] = %d\n", i, *(int *)arr[i]);
        free(arr[i]);
    }
    free(arr);
}

/* ===================================================================== */
static void demo_vtable(void) {
    section("via LIST_VTABLE");
    List *list = LIST_VTABLE.make();
    if (!list) return;

    for (int i = 0; i < 5; i++)
        LIST_VTABLE.put(list, mkint(i * i));
    LIST_VTABLE.print(list);
    printf("len = %zu\n", LIST_VTABLE.len(list));

    int *pv = (int *)LIST_VTABLE.pop(list);
    printf("popped = %d\n", pv ? *pv : -1);
    free(pv);

    LIST_VTABLE.free(list);
}

/* ===================================================================== */
int main(void) {
    demo_ops();
    demo_make_sz();
    demo_clear_fn();
    demo_nuke();
    demo_free_get();
    demo_vtable();
    return 0;
}


#if 0

#include "input.h"


int list_main(void) {
    List *list = list_make();
    if (!list) {
        err("list creation failed");
        return 1;
    }

    int inp = inp_int_s("how much input: ", 3, 100);

    for (int i = 0; i < inp; i++) {
        int tmp = inp_int();
        int *val = (int*)malloc(sizeof(tmp));
        if (!val) {
            err("malloc failed on item");
            goto free;
        }
        *val = tmp;
        if (!list_put(list, val)) {
            free(val);
            err("failed put in list");
            goto free;
        }
    }
    list_print(list);

    list_free(list);
    list = NULL;
    return 0;
free:
    list_free(list);
    list = NULL;
    return 1;
}
int list_r_main(void) {
    List list = {0};
    if (!LIST_VTABLE.init(&list)) {
        err("list creation failed");
        return 1;
    }

    int inp = inp_int_s("how much input: ", 3, 100);
    if (!LIST_VTABLE.resize(&list, inp, sizeof(int))) {
        err("could not allocate memory");
        goto free;
    }
    goto good;

free:
    LIST_VTABLE.nuke(&list);
    return 1;
}
#endif


