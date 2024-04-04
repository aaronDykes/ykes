#ifndef _VIRTUAL_MACHINE_H
#define _VIRTUAL_MACHINE_H

#include "debug.h"
#include "arena_table.h"

typedef enum
{
    INTERPRET_SUCCESS,
    INTERPRET_COMPILE_ERR,
    INTERPRET_RUNTIME_ERR

} Interpretation;

struct CallFrame
{
    Closure *closure;
    uint8_t *ip;
    uint8_t *ip_start;
    Stack *slots;
};

struct vm
{
    int frame_count;
    int garbage_count;
    int garbage_len;
    int argc;
    int cargc;
    CallFrame frames[FRAMES_MAX];
    Stack *stack;
    Stack *call_stack;
    Stack *class_stack;
    Stack *native_calls;
    Upval *open_upvals;
    Table *glob;
};

vm machine;

void initVM();
void freeVM();

Interpretation run();
Interpretation interpret(const char *source);

#endif