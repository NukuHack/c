
#ifndef TINYFS_H
#define TINYFS_H

#include <stddef.h>
#include <stdbool.h>

#define TF_NAME_MAX 260   /* max filename length (fits Windows MAX_PATH) */

struct tf_dirent {
    char name[TF_NAME_MAX];   /* filename only, no path */
    bool is_dir;              /* if directory (not through a symlink) */
    bool is_file;             /* if regular file */
    bool is_link;             /* if symlink / reparse point */
};

typedef struct tf_dir tf_dir;   /* hidden, since os-dependent */

/* Fill buf with the current working directory. */
bool tf_getcwd(char *buf, size_t size);

/* Open a directory for iteration. Returns NULL on error. */
tf_dir *tf_opendir(const char *path);

/* Read the next entry. Return 0-noerr, 1-err */
bool tf_readdir(tf_dir *d, struct tf_dirent *ent);

/* Close and free the handle. Safe on NULL. */
void tf_closedir(tf_dir *d);

/* Store the parent directory of `path` into `out`.
 * If `path` is a filesystem root ("/ " or "C:\ "), it is used as-is
 * (there is nothing above root, so we must not fail). */
void parent_dir(const char *path, char *out, size_t outsz);

#endif /* TINYFS_H */

