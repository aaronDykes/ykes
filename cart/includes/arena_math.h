#ifndef _ARENA_MATH_H
#define _ARENA_MATH_H
#include "arena_memory.h"

arena _neg(arena n);
arena _add(arena a, arena b);
arena _sub(arena a, arena b);
arena _mul(arena a, arena b);
arena _div(arena a, arena b);
arena _mod(arena a, arena b);
arena _eq(arena a, arena b);
arena _ne(arena a, arena b);
arena _seq(arena a, arena b);
arena _sne(arena a, arena b);
arena _lt(arena a, arena b);
arena _le(arena a, arena b);
arena _gt(arena a, arena b);
arena _ge(arena a, arena b);
arena _or(arena a, arena b);
arena _and(arena a, arena b);
arena _inc(arena b);
arena _dec(arena b);

element _get_access(element a, element b);
element _get_each_access(element a, int index);
void _set_access(element val, arena index, element el);
element _push_array_val(element val, element el);
element _pop_array_val(element val);

arena _len(element el);

arena _sqr(arena a);
arena _prime(arena a);

#endif
