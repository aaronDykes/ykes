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

#endif
