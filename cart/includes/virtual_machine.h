#ifndef _VIRTUAL_MACHINE_H
#define _VIRTUAL_MACHINE_H

#include "debug.h"
#include "table.h"
#include <limits.h>

#define FRAMES_MAX 500

typedef enum
{
    INTERPRET_SUCCESS,
    INTERPRET_COMPILE_ERR,
    INTERPRET_RUNTIME_ERR

} Interpretation;

typedef struct CallFrame CallFrame;
typedef struct state state;
typedef struct vm_stack vm_stack;
typedef struct vm vm;

struct CallFrame
{
    closure *closure;
    uint8_t *ip;
    uint8_t *ip_start;
    element *slots;
    uint8_t return_index;
};

struct state
{
    uint16_t frame;
    uint8_t argc;
    uint8_t cargc;
};

struct vm_stack
{
    stack *main;
    stack *obj;
};

struct vm
{
    state count;

    CallFrame frames[FRAMES_MAX];
    vm_stack stack;

    element pop_val;
    instance *current_instance;
    table *init_fields;
    upval *open_upvals;
    table *glob;
};

static vm machine;

void initVM(void);
void freeVM(void);

Interpretation run(void);
Interpretation interpret(const char *source);
Interpretation interpret_path(const char *source, const char *path, const char *name);

#endif
