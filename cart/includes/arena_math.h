#ifndef _ARENA_MATH_H
#define _ARENA_MATH_H
#include "arena_memory.h"

Arena _neg(Arena n);
Arena _add(Arena a, Arena b);
Arena _sub(Arena a, Arena b);
Arena _mul(Arena a, Arena b);
Arena _div(Arena a, Arena b);
Arena _mod(Arena a, Arena b);
Arena _eq(Arena a, Arena b);
Arena _ne(Arena a, Arena b);
Arena _seq(Arena a, Arena b);
Arena _sne(Arena a, Arena b);
Arena _lt(Arena a, Arena b);
Arena _le(Arena a, Arena b);
Arena _gt(Arena a, Arena b);
Arena _ge(Arena a, Arena b);
Arena _or(Arena a, Arena b);
Arena _and(Arena a, Arena b);
Arena _inc(Arena b);
Arena _dec(Arena b);

Arena _access(Arena a, Arena b);

Arena _sqr(Arena a);
Arena _prime(Arena a);

#endif
