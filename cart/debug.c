#include <stdio.h>
#include "debug.h"

static int simple_instruction(const char *name, int offset)
{
    printf("%s\n", name);
    return ++offset;
}

static int constant_instruction(const char *name, Chunk c, int offset)
{
    uint8_t constant = c->op_codes.as.Bytes[offset + 1];

    printf("%-16s %4d '", name, constant);
    print(c->constants.vals[constant]);
    printf("\n");
    return offset + 2;
}

void disassemble_chunk(Chunk c, const char *name)
{

    printf("==== chunk: `%s` ====\n", name);

    c->line = 1;
    for (int i = 0; i < c->count;)
        i = disassemble_instruction(c, i);
}

int disassemble_instruction(Chunk c, int offset)
{

    printf("%d: %04d ", c->line++, offset);

    switch (c->op_codes.as.Bytes[offset])
    {
    case OP_CONSTANT:
        return constant_instruction("OP_CONSTANT", c, offset);
    case OP_NEG:
        return simple_instruction("OP_NEGATE", offset);
    case OP_ADD:
        return simple_instruction("OP_ADD", offset);
    case OP_SUB:
        return simple_instruction("OP_SUBTRACT", offset);
    case OP_MUL:
        return simple_instruction("OP_MULTIPLY", offset);
    case OP_DIV:
        return simple_instruction("OP_DIVIDE", offset);
    case OP_MOD:
        return simple_instruction("OP_MOD", offset);
    case OP_EQ:
        return simple_instruction("OP_EQ", offset);
    case OP_LT:
        return simple_instruction("OP_LT", offset);
    case OP_LE:
        return simple_instruction("OP_LE", offset);
    case OP_GT:
        return simple_instruction("OP_GT", offset);
    case OP_GE:
        return simple_instruction("OP_GE", offset);
    case OP_NE:
        return simple_instruction("OP_NE", offset);
    case OP_AND:
        return simple_instruction("OP_AND", offset);
    case OP_OR:
        return simple_instruction("OP_OR", offset);
    case OP_NULL:
        return simple_instruction("OP_NULL", offset);
    case OP_NOOP:
        return ++offset;
    case OP_GET_GLOBAL:
        return simple_instruction("OP_GET_GLOBAL", offset);
    case OP_SET_GLOBAL:
        return simple_instruction("OP_SET_GLOBAL", offset);
    case OP_GLOBAL_DEF:
        return simple_instruction("OP_GLOBAL_DEF", offset);
    case OP_PRINT:
        return simple_instruction("OP_PRINT", offset);
    case OP_RETURN:
        return simple_instruction("OP_RETURN", offset);
    default:
        printf("Unkown opcode: %d\n", offset);
        return ++offset;
    }
}
