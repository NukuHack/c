
#ifndef MY_STRING_H
#define MY_STRING_H

#include <stdbool.h>
#include <ctype.h>
#include <string.h>

typedef struct String String;

String* sv_create(void);
void sv_free(String *v);
char* sv_free_get(String *v);
bool sv_push(String *v, char c);
bool sv_reserve(String *v, size_t need);

#endif /* MY_STRING_H */

