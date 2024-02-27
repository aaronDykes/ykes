#ifndef _VM_UTIL_H
#define _VM_UTIL_H
#include "arena_math.h"

static void check_stack_size();
static void reset_stack();

static void push(arena ar);
static arena pop();
static void popn(arena n);

static arena _neg(arena n);
static arena _add(arena a, arena b);
static arena _sub(arena a, arena b);
static arena _mul(arena a, arena b);
static arena _div(arena a, arena b);
static arena _mod(arena a, arena b);
static arena _eq(arena a, arena b);
static arena _ne(arena a, arena b);
static arena _lt(arena a, arena b);
static arena _le(arena a, arena b);
static arena _gt(arena a, arena b);
static arena _ge(arena a, arena b);
static arena _or(arena a, arena b);
static arena _and(arena a, arena b);

static arena find(arena tmp);
static bool exists(arena tmp);

#endif