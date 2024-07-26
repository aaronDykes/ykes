#include "chunk.h"

function *_function(_key name)
{
    function *func = ALLOC(sizeof(function));
    func->arity = 0;
    func->uargc = 0;
    func->name = name;
    init_chunk(&func->ch);

    return func;
}
ip_vector ip_vec(void)
{

    ip_vector ip;
    ip.count = 0;
    ip.len = STACK_SIZE;
    ip.bytes = NULL;
    ip.bytes = ALLOC(sizeof(uint16_t) * ip.len);
    return ip;
}
generic_vector gen_vec(void)
{
    generic_vector g;
    g.count = 0;
    g.len = STACK_SIZE;
    g.bytes = NULL;
    g.bytes = ALLOC(sizeof(uint16_t) * g.len);
    return g;
}

void init_chunk(chunk *c)
{
    c->lines.bytes = NULL;
    c->cases.bytes = NULL;
    c->ip.bytes = NULL;
    c->constants = NULL;

    c->lines = gen_vec();
    c->cases = gen_vec();
    c->ip = ip_vec();
    c->constants = GROW_STACK(NULL, STACK_SIZE);
}

void write_chunk(chunk *c, uint8_t byte, uint16_t line)
{

    size_t size = 0;
    if (c->ip.len < c->ip.count + 1)
    {
        size = c->ip.len * INC;
        c->ip.bytes = REALLOC(c->ip.bytes, c->ip.len, size);
    }

    if (c->lines.len < c->lines.count + 1)
    {
        size = c->lines.len * INC * sizeof(uint16_t);
        c->lines.bytes = REALLOC(c->lines.bytes, c->lines.len * sizeof(uint16_t), size);
    }

    c->lines.bytes[c->lines.count++] = line;
    c->ip.bytes[c->ip.count++] = byte;
}

int add_constant(chunk *c, element ar)
{
    push(&c->constants, ar);
    return c->constants->count - 1;
}
