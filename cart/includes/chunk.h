#ifndef _CHUNK_H
#define _CHUNK_H

#include "value.h"

typedef enum
{
    OP_CONSTANT,
    OP_PRINT,

    OP_POP,
    OP_POPN,
    OP_PUSH,

    OP_GLOBAL_DEF,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,

    OP_GET_LOCAL,
    OP_SET_LOCAL,

    OP_ASSIGN,
    OP_NEG,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_MOD,
    OP_DIV,

    OP_BIT_AND,
    OP_BIT_OR,

    OP_AND,
    OP_OR,

    OP_SEQ,
    OP_SNE,
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,

    OP_ELIF,
    OP_JMPF,
    OP_JMPT,
    OP_JMP,
    OP_LOOP,

    OP_NOOP,
    OP_NULL,

    OP_RETURN
} opcode;

struct chunk
{
    int count;
    int capacity;
    int line;
    arena op_codes;
    value constants;
};

typedef struct chunk chunk;
typedef chunk *Chunk;

void init_chunk(Chunk c);
void write_chunk(Chunk c, uint8_t byte);
int add_constant(Chunk c, arena ar);
void free_chunk(Chunk c);

#endif