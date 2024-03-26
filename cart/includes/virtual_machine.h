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

void initVM();
void freeVM();

void mark_object(Element val);
Interpretation run();
Interpretation interpret(const char *source);

#endif