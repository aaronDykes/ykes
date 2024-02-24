#include "stdlib.h"
#include "chunk.h"

void init_chunk(Chunk c)
{
    c->capacity = 0;
    c->count = 0;
    c->op_codes = arena_init(NULL, 0, ARENA_BYTE_PTR);
    c->line = 0;
    init_value_array(&c->constants);
}

void write_chunk(Chunk c, uint8_t byte)
{

    if (c->capacity < c->count + 1)
    {
        c->capacity = GROW_CAPACITY(c->capacity);
        c->op_codes = GROW_ARRAY(&c->op_codes, c->capacity * sizeof(uint8_t));
    }

    c->op_codes.as.bytes[c->count++] = byte;
}

int add_constant(Chunk c, arena ar)
{
    write_value_array(&c->constants, ar);
    return c->constants.count - 1;
}

void free_chunk(Chunk c)
{
    FREE_ARRAY(&c->op_codes);
    free_value_array(&c->constants);
    init_chunk(c);
}