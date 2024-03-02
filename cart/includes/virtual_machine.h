#ifndef _VIRTUAL_MACHINE_H
#define _VIRTUAL_MACHINE_H

#include "debug.h"
#include "table.h"

typedef enum
{
    INTERPRET_SUCCESS,
    INTERPRET_COMPILE_ERR,
    INTERPRET_RUNTIME_ERR

} Interpretation;

struct vm
{
    Chunk ch;
    arena ip;
    uint8_t *ip_start;
    int max_size;
    int current_size;
    arena *stack;
    arena *stack_top;
    dict d;
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