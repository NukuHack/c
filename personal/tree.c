/* tree.c - tinyfs based "tree" utility (using the small list library)
 *
 * usage:  tree [path] [maxdepth]      (maxdepth 0 / omitted = unlimited)
 */
#include "tinyfs.h"
#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ---------------- comparators ---------------- */

static int ci_cmp(const char *a, const char *b) {
    for (;; a++, b++) {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        if (!ca) return 0;
    }
}

/* elements in the list are void*, so pa/pb point at void* */
static int ent_cmp(const void *pa, const void *pb) {
    const struct tf_dirent *a = *(const struct tf_dirent * const *)pa;
    const struct tf_dirent *b = *(const struct tf_dirent * const *)pb;
    if (a->is_dir != b->is_dir)          /* directories first */
        return a->is_dir ? -1 : 1;
    return ci_cmp(a->name, b->name);
}

/* fills *out with heap-allocated dirents (caller owns via list_nuke)
 * returns false if the directory could not be opened,
 * or if an allocation / list_put failed part-way through. */
static bool list_dir(const char *path, List *out) {
    tf_dir *d = tf_opendir(path);
    if (!d) return false;

    struct tf_dirent e;
    while (tf_readdir(d, &e)) {
        struct tf_dirent *p = (struct tf_dirent *)malloc(sizeof *p);
        if (!p) goto fail;
        *p = e;
        if (!list_put(out, p)) {
            free(p);
            goto fail;
        }
    }
    tf_closedir(d);
    goto good;
fail:
    tf_closedir(d);
    return false;
good:
    void **arr = list_get_all(out);
    qsort(arr, list_len(out), sizeof *arr, ent_cmp);
    return true;
}

/* ---------------- path joining ---------------- */

static char *path_join(const char *dir, const char *name) {
    size_t dl = strlen(dir), nl = strlen(name);
    char *p = (char *)malloc(dl + nl + 2);
    if (!p) return NULL;
    memcpy(p, dir, dl);
    if (dl && dir[dl-1] != '/' && dir[dl-1] != '\\')
        p[dl++] = '/';                   /* Windows accepts '/' too */
    memcpy(p + dl, name, nl + 1);
    return p;
}

/* ---------------- recursive walk ---------------- */

static void walk(const char *path, const char *prefix, int depth, int maxdepth, size_t *ndirs, size_t *nfiles)
{
    List l = {0};
    if (!list_init(&l)) return;

    if (!list_dir(path, &l)) {
        list_nuke(&l);
        return;
    }

    size_t n = list_len(&l);
    for (size_t i = 0; i < n; i++) {
        const struct tf_dirent *e = (const struct tf_dirent *)list_get(&l, i);
        if (!e) continue;
        int last = (i + 1 == n);

        const char *mark = e->is_dir ? "/" : (e->is_link ? "@" : "");
        printf("%s%s%s%s\n", prefix, last ? "`-- " : "|-- ", e->name, mark);

        if (e->is_dir) {
            (*ndirs)++;
            if (maxdepth && depth >= maxdepth) {
                continue;
            }

            char *child = path_join(path, e->name);
            if (child) {
                size_t plen = strlen(prefix);
                char  *npre = (char *)malloc(plen + 5);
                if (npre) {
                    memcpy(npre, prefix, plen);
                    memcpy(npre + plen, last ? "    " : "|   ", 5); /* incl. NUL */
                    walk(child, npre, depth + 1, maxdepth, ndirs, nfiles);
                    free(npre);
                }
                free(child);
            }
        } else {
            (*nfiles)++;
        }
    }
    list_nuke(&l);
}

/* ---------------- main ---------------- */

int main(int argc, char **argv) {
    char cwd[4096];
    if (!tf_getcwd(cwd, sizeof cwd)) {
        fprintf(stderr, "tree: cannot determine current directory\n");
        return 1;
    }

    char parent[4096];
    parent_dir(cwd, parent, sizeof parent);

    const char *root = (argc > 1) ? argv[1] : parent;
    int maxdepth = (argc > 2) ? atoi(argv[2]) : 0;
    if (maxdepth < 0) maxdepth = 0;

    printf("cwd:  %s\n", cwd);
    printf("root: %s\n", root);

    size_t ndirs = 0, nfiles = 0;
    walk(root, "", 1, maxdepth, &ndirs, &nfiles);

    printf("\n%zu director%s, %zu file%s\n",
           ndirs,  ndirs  == 1 ? "y" : "ies",
           nfiles, nfiles == 1 ? ""  : "s");
    return 0;
}

