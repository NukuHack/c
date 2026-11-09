#include "logging.h"
#include "my_utils.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

log_lvl log_level_global = DEBUG;
#if defined(LOG_FILE)
char *log_file = LOG_FILE;
#else
char *log_file = NULL;
#endif

bool color = true;

// in a real logging stuff i would make the FILE* not close at each write
// and use the buffering (or a custom buffer+flush) system to make it better
int log_main(void) {
#if !defined(LOG_FILE)
    char *tmp = temp_dir();
    char file[] = "/latest.log";
    int tmp_l = strlen(tmp);
    int file_l = strlen(file);
    log_file = (char*)malloc(sizeof(char) * (tmp_l + file_l + 1));
    if (!log_file) {
        err("malloc fail");
        return 1;
    }
    memcpy(log_file, tmp, tmp_l);
    free(tmp);
    memcpy(log_file + tmp_l, file, file_l + 1);
#endif
    log_clear();
    log_g("hello world");
    log_g("some more");
    log_l(ERROR, "big bad");
    log_setlvl(WARN);
    log_g("some more");

#if !defined(LOG_FILE)
    free(log_file);
#endif

    return 0;
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

void log_g(const char *msg) {
    log_l(log_level_global, msg);
}


void log_clear(void) {
    FILE *log_f = fopen(log_file, "w"); // write, creat if non exist, clear if it does
    if (!log_f) {
        perror("log file");
        return;
    }
    fclose(log_f);
}
void log_l(log_lvl lvl, const char *msg) {
    FILE *log_f = fopen(log_file, "a"); // append, creat if non exist
    if (!log_f) {
        perror("log file");
        return;
    }

    if (color)
        fputs(loglvl_col(lvl), log_f);

    fputs(loglvl_str(lvl), log_f);
    fputs(": ", log_f);

    int len = 0;
    if (msg && (len = strlen(msg)) > 0) {
        bool endN = msg[len-1] == '\n';
        fputs(msg, log_f);
        if (!endN)
            fputc('\n', log_f);
    }

    if (color)
        fputs("\033[0m", log_f);

    if (fflush(log_f) != 0)
        perror("log file write");

    // correct ending :
    fclose(log_f);
    return;
}


void log_setlvl(log_lvl lvl) {
    log_level_global = lvl;
}


