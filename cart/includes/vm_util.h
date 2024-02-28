#ifndef _VM_UTIL_H
#define _VM_UTIL_H
#include "arena_math.h"

static void check_stack_size();
static void reset_stack();

static void push(arena ar);
static void popn(arena n);

static arena find(arena tmp);
static bool exists(arena tmp);

#endif