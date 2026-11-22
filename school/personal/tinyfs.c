
#include "tinyfs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if defined(_WIN32)
#  include <windows.h>
#  include <direct.h>
#else
#  include <unistd.h>
#  include <dirent.h>
#  include <sys/stat.h>
#endif

#define TF_PATH_MAX 4096

struct tf_dir {
#if defined(_WIN32)
    HANDLE             h;
    WIN32_FIND_DATAA   fd;
    int                started;
    int                done;
#else
    DIR               *d;
#endif
    char path[TF_PATH_MAX];
};

/* ---------- getcwd ---------- */
bool tf_getcwd(char *buf, size_t size) {
#if defined(_WIN32)
    return _getcwd(buf, (int)size);
#else
    return getcwd(buf, size);
#endif
}

/* ================= Windows ================= */
#if defined(_WIN32)

tf_dir *tf_opendir(const char *path) {
    if (!path || !*path) return NULL;
    tf_dir *d = (tf_dir*)calloc(1, sizeof(*d));
    if (!d) return NULL;

    size_t n = strlen(path);
    if (n + 3 > sizeof(d->path)) { free(d); return NULL; }
    memcpy(d->path, path, n);
    while (n > 1 && (d->path[n-1] == '/' || d->path[n-1] == '\\')) n--;
    d->path[n] = '\0';

    if (n > 0 && d->path[n-1] != '\\' && d->path[n-1] != '/') {
        d->path[n++] = '\\';
    }
    d->path[n++] = '*';
    d->path[n]   = '\0';

    d->h = FindFirstFileA(d->path, &d->fd);
    if (d->h == INVALID_HANDLE_VALUE) { free(d); return NULL; }
    d->started = 0;
    d->done    = 0;
    return d;
}

bool tf_readdir(tf_dir *d, struct tf_dirent *ent) {
    if (!d || d->done) return false;
    while(1) {
        const char *name;
        DWORD attr;
        if (!d->started) {
            d->started = 1;
            name = d->fd.cFileName;
            attr = d->fd.dwFileAttributes;
        } else {
            if (!FindNextFileA(d->h, &d->fd)) { d->done = 1; return false; }
            name = d->fd.cFileName;
            attr = d->fd.dwFileAttributes;
        }
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;

        strncpy(ent->name, name, TF_NAME_MAX - 1);
        ent->name[TF_NAME_MAX - 1] = '\0';
        ent->is_dir = ent->is_file = ent->is_link = 0;

        if (attr & FILE_ATTRIBUTE_REPARSE_POINT)
            ent->is_link = 1;
        else if (attr & FILE_ATTRIBUTE_DIRECTORY)
            ent->is_dir = 1;
        else
            ent->is_file = 1;

        return true;
    }
}

void tf_closedir(tf_dir *d) {
    if (!d) return;
    if (d->h && d->h != INVALID_HANDLE_VALUE) FindClose(d->h);
    free(d);
}

/* ================= POSIX (Linux / macOS / BSD) ================= */
#else

tf_dir *tf_opendir(const char *path) {
    if (!path || !*path) return NULL;
    tf_dir *d = (tf_dir*)calloc(1, sizeof(*d));
    if (!d) return NULL;

    d->d = opendir(path);
    if (!d->d) { free(d); return NULL; }

    size_t n = strlen(path);
    if (n >= sizeof(d->path)) n = sizeof(d->path) - 1;
    memcpy(d->path, path, n);
    d->path[n] = '\0';
    return d;
}

bool tf_readdir(tf_dir *d, struct tf_dirent *ent) {
    if (!d) return false;
    struct dirent *de;
    for (;;) {
        de = readdir(d->d);
        if (!de) return false;
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;

        strncpy(ent->name, de->d_name, TF_NAME_MAX - 1);
        ent->name[TF_NAME_MAX - 1] = '\0';
        ent->is_dir = ent->is_file = ent->is_link = 0;

#ifdef DT_DIR
        if (de->d_type == DT_DIR) {
            ent->is_dir  = 1; return true;
        }
        if (de->d_type == DT_REG) {
            ent->is_file = 1; return true;
        }
        if (de->d_type == DT_LNK) {
            ent->is_link = 1; return true;
        }
        if (de->d_type != DT_UNKNOWN) return true;   /* fifo, sock, ... : all 0 */
#endif
        /* Fallback: DT_UNKNOWN or no d_type (e.g. some network FS) */
        struct stat st;
        char full[TF_PATH_MAX];
        int n = snprintf(full, sizeof(full), "%s/%s", d->path, de->d_name);
        if (n > 0 && (size_t)n < sizeof(full) && lstat(full, &st) == 0) {
            if (S_ISDIR(st.st_mode)) ent->is_dir  = 1;
            else if (S_ISREG(st.st_mode)) ent->is_file = 1;
            else if (S_ISLNK(st.st_mode)) ent->is_link = 1;
        }
        return true;
    }
}

void tf_closedir(tf_dir *d) {
    if (!d) return;
    if (d->d) closedir(d->d);
    free(d);
    d = NULL;
}

#endif


void parent_dir(const char *path, char *out, size_t outsz) {
    size_t n = strlen(path);
    /* drop trailing separators, but keep a lone root "/" or "C:\" */
    while (n > 1 && (path[n-1] == '/' || path[n-1] == '\\')) n--;

#if defined(_WIN32)
    /* drive root: "C:" or "C:\" — nothing above it */
    if (n == 2 && path[1] == ':') {
        snprintf(out, outsz, "%c:\\", path[0]);
        return;
    }
    /* back up to the last separator */
    while (n > 0 && path[n-1] != '/' && path[n-1] != '\\') n--;
    /* drop the separator itself, but keep drive root "C:\" */
    while (n > 1 && (path[n-1] == '/' || path[n-1] == '\\')) n--;
    if (n == 2 && path[1] == ':') n = 3;   /* keep "C:\" */
    if (n == 0) {                          /* relative, no separator */
        snprintf(out, outsz, ".");
        return;
    }
    snprintf(out, outsz, "%.*s", (int)n, path);
#else
    /* POSIX root */
    if (n == 1 && path[0] == '/') {
        snprintf(out, outsz, "/");
        return;
    }
    /* back up to the last separator */
    while (n > 0 && path[n-1] != '/') n--;
    /* drop the separator itself, but keep root "/" */
    while (n > 1 && path[n-1] == '/') n--;
    if (n == 0) {                          /* relative, no separator */
        snprintf(out, outsz, ".");
        return;
    }
    snprintf(out, outsz, "%.*s", (int)n, path);
#endif
}
