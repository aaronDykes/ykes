#ifndef _ARENA_STRING_H
#define _ARENA_STRING_H
#include "arena_memory.h"

arena ltoa_eqcmp(long long int llint, arena ar);
arena ltoa_neqcmp(long long int llint, arena ar);
arena itoa_eqcmp(int ival, arena ar);
arena itoa_neqcmp(int ival, arena ar);

void log_err(const char *format, ...);
void str_swap(char *from, char *to);
void string_rev(char *c);
int intlen(int n);
int longlen(long long int n);
char *itoa(char *c, int n);
char *lltoa(char *c, long long int n);

arena prepend_int_to_str(arena s, arena a);
arena prepend_char_to_str(arena s, arena a);
arena prepend_long_to_str(arena s, arena a);

arena append(arena s, arena ar);

arena string_eq(arena s, arena c);
arena string_ne(arena s, arena c);
arena string_gt(arena s, arena c);
arena string_ge(arena s, arena c);
arena string_lt(arena s, arena c);
arena string_le(arena s, arena c);

#endif