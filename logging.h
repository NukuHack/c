
#ifndef LOGGING_H
#define LOGGING_H

#include <stdlib.h>

typedef enum {
    TRACE = 1,
    DEBUG,
    INFO,
    WARN,
    ERROR
} log_lvl;

#if !defined(LOGGING)
#  define LOGGING 1
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

#if (LOGGING %10 <= 5 && LOGGING != 15)  || LOGGING == -5
#  define LOG_TRACE(...)  log_msg(TRACE, __FILE__, __LINE__, __VA_ARGS__)
#else
#  define LOG_TRACE(...)  ((void)0)
#endif
#if (LOGGING %10 <= 4 && LOGGING != 14)  || LOGGING == -4
#  define LOG_DEBUG(...)  log_msg(DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#else
#  define LOG_DEBUG(...)  ((void)0)
#endif
#if (LOGGING %10 <= 3 && LOGGING != 13)  || LOGGING == -3
#  define LOG_INFO(...)  log_msg(INFO, __FILE__, __LINE__, __VA_ARGS__)
#else
#  define LOG_INFO(...)  ((void)0)
#endif
#if (LOGGING %10 <= 2 && LOGGING != 12)  || LOGGING == -2
#  define LOG_WARN(...)  log_msg(WARN, __FILE__, __LINE__, __VA_ARGS__)
#else
#  define LOG_WARN(...)  ((void)0)
#endif
#if (LOGGING %10 <= 1 && LOGGING != 11)  || LOGGING == -1
#  define LOG_ERROR(...)  log_msg(ERROR, __FILE__, __LINE__, __VA_ARGS__)
#else
#  define LOG_ERROR(...)  ((void)0)
#endif

// Variable length argruements !! so nice
void log_msg(log_lvl lvl, const char *file, int line, const char *fmt, ...);
void log_g(const char *msg);
void log_l(log_lvl lvl, const char *msg);
void log_setlvl(log_lvl lvl);
void log_clear(void);
const char *loglvl_str(log_lvl lvl);
char *time_now(char *buf, size_t n);

extern log_lvl log_level_global;

#endif /* LOGGING_H */ 

