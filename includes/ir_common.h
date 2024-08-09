#ifndef _IR_COMMON_H
#define _IR_COMMON_H
#include "parser.h"

typedef struct ir ir;

struct ir
{
	uint8_t    obj;
	table     *lookup;
	ast_stack *ast;
};

#endif
