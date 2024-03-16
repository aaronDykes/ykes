#ifndef _VIRTUAL_MACHINE_H
#define _VIRTUAL_MACHINE_H

#include "debug.h"
#include "table.h"
#include "compiler.h"

#define FRAMES_MAX 500

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
typedef struct CallFrame CallFrame;

struct vm
{
    CallFrame frames[FRAMES_MAX];
    int frame_count;
    Stack *stack;
    Stack *call_stack;
    Stack *native_calls;
    Upval *open_upvals;
    dict glob;
};
typedef struct vm vm;
typedef vm *Vm;

static vm machine;

void initVM();
void freeVM();

Interpretation run();
Interpretation interpret(const char *source);

#endif