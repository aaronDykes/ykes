#ifndef _VIRTUAL_MACHINE_H
#define _VIRTUAL_MACHINE_H

#include "debug.h"
#include "table.h"

struct vm
{
    Chunk ch;
    arena ip;
    int max_size;
    int current_size;
    arena *stack;
    arena *stack_top;
    dict d;
};

typedef enum
{
    INTERPRET_SUCCESS,
    INTERPRET_COMPILE_ERR,
    INTERPRET_RUNTIME_ERR

} Interpretation;

typedef struct vm vm;
typedef vm *Vm;

void initVM();
void freeVM();

void push(arena v);

Interpretation run();
Interpretation interpret(const char *source);

#endif