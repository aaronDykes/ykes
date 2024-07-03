#ifndef _VIRTUAL_MACHINE_H
#define _VIRTUAL_MACHINE_H

#include "debug.h"
#include "arena_table.h"
#include <limits.h>

typedef enum
{
    INTERPRET_SUCCESS,
    INTERPRET_COMPILE_ERR,
    INTERPRET_RUNTIME_ERR

} Interpretation;

typedef struct CallFrame CallFrame;
typedef struct vm vm;

struct CallFrame
{
    closure *closure;
    uint8_t *ip;
    uint8_t *ip_start;
    stack *slots;
};

struct vm
{
    int frame_count;
    int argc;
    int cargc;

    CallFrame frames[FRAMES_MAX];
    stack *stack;
    stack *call_stack;
    stack *method_call_stack;
    stack *class_stack;
    stack *native_calls;
    element pop_val;
    upval *open_upvals;
    table *glob;
};

vm machine;

void initVM(void);
void freeVM(void);

Interpretation run(void);
Interpretation interpret(const char *source);
Interpretation interpret_path(const char *source, const char *path, const char *name);

#endif
