
#ifndef LOGGING_H
#define LOGGING_H

typedef enum {
    TRACE = 1,
    DEBUG,
    INFO,
    WARN,
    ERROR
} log_lvl;

void log_g(const char *msg);
void log_l(log_lvl lvl, const char *msg);
void log_setlvl(log_lvl lvl);
void log_clear(void);
const char *loglvl_str(log_lvl lvl);

extern log_lvl log_level_global;

#endif /* LOGGING_H */ 

