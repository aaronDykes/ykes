#ifndef _VM_UTIL_H
#define _VM_UTIL_H
#include "arena_math.h"

struct Runtime
{
    arena ar;
    int true_count;
    uint16_t shorty;
};

typedef struct Runtime Runtime;

static void check_stack_size();
static void reset_stack();

static void init_runtime(Runtime *runtime);
static void push(arena ar);
static void popn(arena n);

static arena find(arena tmp);
static bool exists(arena tmp);

#endif