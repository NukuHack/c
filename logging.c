#define _POSIX_C_SOURCE 200809L
#include "logging.h" // self
#include "my_utils.h" // utils
#include <errno.h> // err
#include <stdio.h> // basic io
#include <stdbool.h> // bool
#include <string.h> // mem op
#include <stdarg.h> // va-arg

/* local time */
#if defined(_WIN32)
#  include <windows.h>
#else
#  include <unistd.h>
#  include <sys/time.h>
#endif
#include <time.h> // time

/* buffer size */
#if !defined(KB)
#  define KB 1024
#endif

#if defined(LOG_FILE)
char *log_file = LOG_FILE;
#else
char *log_file = NULL;
#endif

log_lvl log_level_global = DEBUG;
bool show_color_file = false;
bool show_color = true;
bool show_time = true;
bool use_file = true;
bool use_stderr = true;

void log_set_global(log_lvl _lvl, bool _show_color, bool _show_color_file, bool _show_time, bool _use_file, bool _use_stderr) {
    log_level_global = _lvl;
    show_color_file = _show_color;
    show_color = _show_color_file;
    show_time = _show_time;
    use_file = _use_file;
    use_stderr = _use_stderr;
}
void log_setlvl(log_lvl _lvl) {
    log_level_global = _lvl;
}

// in a real logging stuff i would make the FILE* not close at each write
// and use the buffering (or a custom buffer+flush) system to make it better
int log_main(void) {
#if !defined(LOG_FILE)
    char *file = "/latest.log",
         *tmp = temp_dir();
    int tmp_l = strlen(tmp),
        file_l = strlen(file);
    log_file = (char*)malloc(sizeof(char) * (tmp_l + file_l + 1));
    if (!log_file) {
        perror("malloc logfile");
        return 1;
    }
    memcpy(log_file, tmp, tmp_l);
    free(tmp);
    memcpy(log_file + tmp_l, file, file_l + 1);
#endif
    log_clear();
    LOG_WARN("hello world");
    LOG_DEBUG("stuff");
    LOG_DEBUG("some more");
    log_line(ERROR, "big bad");
    log_setlvl(WARN);
    LOG_DEBUG("sme hidden");
    LOG_WARN("Important, %s", "trace : NONE");
    LOG_ERROR("some more");
    LOG_ERROR("Fatal %d", 69);

#if !defined(LOG_FILE)
    free(log_file);
#endif

    return 0;
}

void log_clear(void) {
    FILE *log_f = fopen(log_file, "w"); // write, creat if non exist, clear if it does
    if (!log_f) {
        perror("log file");
        return;
    }
    fclose(log_f);
}


const char *loglvl_col(log_lvl lvl) {
    switch (lvl) {
        case TRACE: return "\033[32m";
        case DEBUG: return "\033[34m";
        case INFO: return "\033[27m";
        case WARN: return "\033[33m";
        case ERROR: return "\033[31m";
    }
    return "";
}
const char *loglvl_str(log_lvl lvl) {
    switch (lvl) {
        case TRACE: return "TRACE";
        case DEBUG: return "DEBUG";
        case INFO: return "INFO";
        case WARN: return "WARN";
        case ERROR: return "ERROR";
    }
    return "UNKNOWN";
}

void log_line(log_lvl lvl, const char *msg) {
    if (lvl > log_level_global)
        return;

    static char t_arr[24] = {0};
    if (show_time)
        time_now(t_arr, 24);

    int len = 0;
    bool endN = true; // ends with new line
    if (msg && (len = strlen(msg)) > 0)
        endN = msg[len-1] == '\n';

    if (use_stderr) { // to stderr
        FILE *out = stderr;
        if (show_time) {
            fputs(t_arr, out);
            fputc(' ', out);
        }
        if (show_color)
            fputs(loglvl_col(lvl), out);

        fputs(loglvl_str(lvl), out);
        fputs(": ", out);

        if (len > 0) {
            fputs(msg, out);
            if (!endN)
                fputc('\n', out);
        }

        if (show_color)
            fputs("\033[0m", out);

        if (fflush(out) != 0)
            perror("log flush");
    }
    if (use_file) {
        FILE *out = fopen(log_file, "a"); // append, creat if non exist
        if (!out) {
            perror("log file");
            use_file = false; // since we probably will not be able to make a log file there
            return;
        }

        if (show_time) {
            fputs(t_arr, out);
            fputc(' ', out);
        }
        if (show_color_file)
            fputs(loglvl_col(lvl), out);

        fputs(loglvl_str(lvl), out);
        fputs(": ", out);

        if (len > 0) {
            fputs(msg, out);
            if (!endN)
                fputc('\n', out);
        }

        if (show_color_file)
            fputs("\033[0m", out);

        if (fclose(out) != 0) // no need to flush, close flushes for us
            perror("log file write");
    }
}

void log_msg(log_lvl lvl, const char *file, int line, const char *fmt, ...) {
    char buf[KB]; // KB of buffer, if more needed ... BRUH

    // First format the file and line part
    int written = snprintf(buf, sizeof(buf), "%s : line:%d ; ", file, line);
    if (written < 0 || (size_t)written >= sizeof(buf)) {
        return; // Buffer too small
    }

    // Then append the formatted message using va_args
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf + written, sizeof(buf) - written, fmt, args);
    va_end(args);

    log_line(lvl, buf);
}

/* Writes "YYYY-MM-DDTHH:MM:SS.mmm" (local time) into buf, needs 24 bytes. "NULL" = err */
char *time_now(char *buf, size_t n) {
    time_t sec; long ms; struct tm tm;

#define ULL unsigned long long
#if defined(_WIN32)
    FILETIME ft; // imagine still using only 32 bits ...
    GetSystemTimeAsFileTime(&ft);
    ULL t100 =                       /* 100 ns ticks since 1601 */
        ((ULL)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    t100 -= 116444736000000000ULL;   /* rebase 1601 -> 1970 */

    ULL ms_total = t100 / 10000ULL;  /* 100ns -> ms */
    sec = (time_t)(ms_total / 1000ULL);
    ms  = (long)(ms_total % 1000ULL);
#  if defined(_MSC_VER)
    localtime_s(&tm, &sec);          /* (tm*, time_t*) */
#  else
    localtime_r(&sec, &tm);          /* MinGW */
#  endif
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    sec = tv.tv_sec;
    ms  = (long)(tv.tv_usec / 1000);
    localtime_r(&sec, &tm);
#endif

    int k = snprintf(buf, n, "%04d-%02d-%02dT%02d:%02d:%02d.%03ld",
                     tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                     tm.tm_hour, tm.tm_min, tm.tm_sec, ms);
    return (k < 0 || (size_t)k >= n) ? NULL : buf;
}


