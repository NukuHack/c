
#ifndef LOGGING_H
#define LOGGING_H

#include <stdlib.h> // null
#include <stdbool.h> // bool 
#include <stddef.h> // size_t

typedef enum {
    TRACE = 5,
    DEBUG = 4,
    INFO = 3,
    WARN = 2,
    ERROR = 1
} log_lvl;

#if !defined(LOGGING)
#  define LOGGING 4 // defined as DEBUG
#endif

/*
 * made it so theoretical max is 10
 * numbers between 1 and 5(current max) 
 *  - simply allow more logging at bigger numbers
 * numbers between -1 and -5(current max)
 *  - simply allow a Single log level
 * numbers between 11 and 15(current max)
 *  - simply sisable a single level (the first rule apply)
 */

#if (LOGGING %10 >= 5 && LOGGING != 15)  || LOGGING == -5
#  define LOG_TRACE(...)  log_msg(TRACE, __FILE__, __LINE__, __VA_ARGS__)
#else
#  define LOG_TRACE(...)  ((void)0)
#endif
#if (LOGGING %10 >= 4 && LOGGING != 14)  || LOGGING == -4
#  define LOG_DEBUG(...)  log_msg(DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#else
#  define LOG_DEBUG(...)  ((void)0)
#endif
#if (LOGGING %10 >= 3 && LOGGING != 13)  || LOGGING == -3
#  define LOG_INFO(...)  log_msg(INFO, __FILE__, __LINE__, __VA_ARGS__)
#else
#  define LOG_INFO(...)  ((void)0)
#endif
#if (LOGGING %10 >= 2 && LOGGING != 12)  || LOGGING == -2
#  define LOG_WARN(...)  log_msg(WARN, __FILE__, __LINE__, __VA_ARGS__)
#else
#  define LOG_WARN(...)  ((void)0)
#endif
#if (LOGGING %10 >= 1 && LOGGING != 11)  || LOGGING == -1
#  define LOG_ERROR(...)  log_msg(ERROR, __FILE__, __LINE__, __VA_ARGS__)
#else
#  define LOG_ERROR(...)  ((void)0)
#endif

// Variable length argruements !! so nice
// format is printed through a std function
void log_msg(log_lvl lvl, const char *file, int line, const char *fmt, ...);
// non formatted, goes to file + stderr by default
// only colored on stderr by default
void log_line(log_lvl lvl, const char *msg);

// sets the global level - checked at runtime
// might remove it in favor of preprocessor
void log_set_global(log_lvl _lvl, bool _show_color, bool _show_color_file,
        bool _show_time, bool _use_file, bool _use_stderr);
void log_setlvl(log_lvl _lvl);

// clears the log file (if exists)
void log_clear(void);

/* small helpers */
const char *loglvl_str(log_lvl lvl);
const char *loglvl_col(log_lvl lvl);

/* buff len >= 24, otherwise bug, local time :P */
char *time_now(char *buf, size_t n);

#endif /* LOGGING_H */ 

