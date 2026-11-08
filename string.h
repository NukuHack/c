
#ifndef MY_STRING_H
#define MY_STRING_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct String String;

String* sv_create(void);
void sv_free(String *v);
char* sv_free_get(String *v);
bool sv_push(String *v, char c);
bool sv_reserve(String *v, size_t need);

bool is_alpha(char inp);
bool is_numeric(char inp);
bool is_alnum(char inp);
char to_upper(char inp);
char to_lower(char inp);

int str_len(const char* str);

char* alphabetic(char* a, char* b);

#endif /* MY_STRING_H */

