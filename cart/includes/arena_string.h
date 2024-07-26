#ifndef _ARENA_STRING_H
#define _ARENA_STRING_H
#include "arena.h"

static void str_swap(char *from, char *to);
static void string_rev(char *c);
static int intlen(int n);
static int longlen(long long int n);
static char *itoa(char *c, int n);
static char *lltoa(char *c, long long int n);

element append(element s, element ar);

element string_eq(element s, element c);
element string_ne(element s, element c);
element string_gt(element s, element c);
element string_ge(element s, element c);
element string_lt(element s, element c);
element string_le(element s, element c);

#endif
