#ifndef _ARENA_STRING_H
#define _ARENA_STRING_H
#include "arena_memory.h"

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