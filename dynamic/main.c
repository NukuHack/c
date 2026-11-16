/* main.c -- tiny cross-platform hot reload demo. C99.
 *
 * Watches a shared library file, reloads it whenever it changes on disk,
 * and calls a function from it every second.
 *
 * Build (POSIX):
 *     cc -shared -fPIC -o lib.so lib.c
 *     cc -o host main.c -ldl
 *
 * Build (macOS):
 *     cc -dynamiclib -o lib.dylib lib.c
 *     cc -o host main.c
 *
 * Build (Windows / MinGW):
 *     gcc -shared -o lib.dll lib.c
 *     gcc -o host.exe main.c
 *
 * Build (Windows / MSVC):
 *     cl /nologo /LD lib.c /Fe:lib.dll
 *     cl /nologo       main.c /Fe:host.exe
 */

#if defined(_MSC_VER)
#  define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <dlfcn.h>
#  include <sys/stat.h>
#  include <unistd.h>
#endif

/* ---------------------------------------------------------------------------
 * Platform layer: sleep
 * ------------------------------------------------------------------------- */
static void do_sleep(int seconds) {
#ifdef _WIN32
    Sleep((DWORD)(seconds * 1000));
#else
    sleep((unsigned int)seconds);
#endif
}

/* ---------------------------------------------------------------------------
 * Per-platform shared library name.
 *
 * POSIX dlopen() only searches LD_LIBRARY_PATH and the system dirs when the
 * argument has no '/' in it -- it never looks in the current directory.
 * Hence the explicit "./" prefix on POSIX.  Windows' LoadLibraryA() already
 * searches the exe dir and cwd, so the bare name is fine there.
 * ------------------------------------------------------------------------- */
#if defined(_WIN32)
#  define LIB_EXT  "dll"
#  define LIB_PATH "lib." LIB_EXT
#elif defined(__APPLE__)
#  define LIB_EXT  "dylib"
#  define LIB_PATH "./lib." LIB_EXT
#else
#  define LIB_EXT  "so"
#  define LIB_PATH "./lib." LIB_EXT
#endif

typedef void (*hello_fn)(void);

/* ---------------------------------------------------------------------------
 * Platform layer: file fingerprint
 *
 * Returns a value that changes whenever the file is rewritten.  0 means the
 * file does not exist (yet).  We combine mtime and size so edits that land
 * inside the same clock second are still noticed.
 * ------------------------------------------------------------------------- */
static unsigned long long file_fingerprint(const char *path)
{
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA info;
    ULARGE_INTEGER u;
    unsigned long long fp;

    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &info))
        return 0;

    u.LowPart  = info.ftLastWriteTime.dwLowDateTime;
    u.HighPart = info.ftLastWriteTime.dwHighDateTime;

    fp  = u.QuadPart;
    fp ^= ((unsigned long long)info.nFileSizeLow ) << 20;
    fp ^= ((unsigned long long)info.nFileSizeHigh) << 52;
    return fp;
#else
    struct stat st;
    unsigned long long fp;

    if (stat(path, &st) != 0)
        return 0;

    fp  = ((unsigned long long)st.st_mtime) << 32;
    fp ^= ((unsigned long long)(st.st_size & 0xFFFFFFFFu));
    return fp;
#endif
}

/* ---------------------------------------------------------------------------
 * Platform layer: dynamic loading (with error reporting)
 * ------------------------------------------------------------------------- */
#ifdef _WIN32

typedef HMODULE lib_handle;

static lib_handle lib_open(const char *path)
{
    lib_handle h = LoadLibraryA(path);
    if (!h)
        fprintf(stderr, "LoadLibraryA(%s): error %lu\n", path, GetLastError());
    return h;
}

static void *lib_sym(lib_handle h, const char *name)
{
    FARPROC p = GetProcAddress(h, name);
    if (!p)
        fprintf(stderr, "GetProcAddress(%s): error %lu\n", name, GetLastError());
    /* Cast through size_t so MSVC doesn't complain about function->void* */
    return (void *)(size_t)p;
}

static void lib_close(lib_handle h)
{
    FreeLibrary(h);
}

#else /* POSIX */

typedef void *lib_handle;

static lib_handle lib_open(const char *path)
{
    lib_handle h = dlopen(path, RTLD_NOW);
    if (!h) {
        const char *e = dlerror();
        fprintf(stderr, "dlopen(%s): %s\n", path, e ? e : "unknown");
    }
    return h;
}

static void *lib_sym(lib_handle h, const char *name)
{
    void *p;
    const char *e;

    dlerror();              /* clear any stale error */
    p = dlsym(h, name);
    e = dlerror();
    if (e) {
        fprintf(stderr, "dlsym(%s): %s\n", name, e);
        return NULL;
    }
    return p;
}

static void lib_close(lib_handle h)
{
    dlclose(h);
}

#endif

/* ---------------------------------------------------------------------------
 * main
 * ------------------------------------------------------------------------- */
int main(void)
{
    const char        *src       = LIB_PATH;
    lib_handle         handle    = NULL;
    hello_fn           say_hello = NULL;
    unsigned long long last_fp   = 0;

#ifdef _WIN32
    /* Windows keeps a loaded DLL locked, so we can never overwrite lib.dll in
     * place while it is mapped.  Solution: copy the freshly built lib.dll to
     * a unique name and load *that*; the user's file stays unlocked. */
    char copy_path[MAX_PATH];
    int  generation = 0;
    copy_path[0] = '\0';
#endif

    printf("Watching %s -- edit + recompile to hot-reload. Ctrl+C to quit.\n", src);

    for (;;) {
        unsigned long long fp = file_fingerprint(src);

        if (fp != 0 && fp != last_fp) {
            last_fp = fp;
            do_sleep(1);        /* let the compiler finish writing */

            /* Unload the previous version */
            if (handle) {
                lib_close(handle);
                handle    = NULL;
                say_hello = NULL;
#ifdef _WIN32
                if (copy_path[0] != '\0') {
                    DeleteFileA(copy_path);
                    copy_path[0] = '\0';
                }
#endif
            }

            /* Load the new version */
#ifdef _WIN32
            generation++;
            sprintf(copy_path, "lib_gen_%d." LIB_EXT, generation);
            if (!CopyFileA(src, copy_path, FALSE)) {
                fprintf(stderr, "CopyFileA(%s -> %s): error %lu\n",
                        src, copy_path, GetLastError());
                last_fp = 0;    /* retry on next tick */
                continue;
            }
            handle = lib_open(copy_path);
#else
            handle = lib_open(src);
#endif

            if (handle)
                say_hello = (hello_fn)lib_sym(handle, "say_hello");

            if (!say_hello) {
                fprintf(stderr, "could not load %s / symbol 'say_hello'\n", src);
                if (handle) { lib_close(handle); handle = NULL; }
                last_fp = 0;    /* retry on next tick */
                continue;
            }
        }

        if (say_hello)
            say_hello();

        do_sleep(1);
    }

    return 0;   /* unreachable */
}
