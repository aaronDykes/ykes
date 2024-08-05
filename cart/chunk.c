#include "chunk.h"

static generic_vector gen_vec(void)
{
	generic_vector g;
	g.count = 0;
	g.len   = STACK_SIZE;
	g.bytes = NULL;
	g.bytes = ALLOC(sizeof(uint16_t) * g.len);
	return g;
}

static void init_chunk(chunk *c)
{
	c->lines     = NULL;
	c->ip        = NULL;
	c->constants = NULL;
	c->len       = STACK_SIZE;
	c->count     = 0;

	c->cases     = gen_vec();
	c->ip        = ALLOC(STACK_SIZE);
	c->lines     = ALLOC(STACK_SIZE * sizeof(uint16_t));
	c->constants = GROW_STACK(NULL, STACK_SIZE);
}

function *_function(_key name)
{
	function *func = ALLOC(sizeof(function));
	func->arity    = 0;
	func->uargc    = 0;
	func->name     = name;
	init_chunk(&func->ch);

	return func;
}

void write_chunk(chunk *c, uint8_t byte, uint16_t line)
{

	size_t size = 0;
	if (c->len < c->count + 1) {
		size     = c->len * INC;
		c->ip    = REALLOC(c->ip, size);
		c->lines = REALLOC(c->lines, size * sizeof(uint16_t));
		c->len *= INC;
	}
	if (c->cases.len < c->cases.count + 1) {
		size_t size    = c->cases.len * INC * sizeof(uint16_t);
		c->cases.bytes = REALLOC(c->cases.bytes, size);
		c->cases.len *= INC;
	}

	*(c->lines + c->count) = line;
	*(c->ip + c->count++)  = byte;
}

int add_constant(chunk *c, element ar)
{
	push(&c->constants, ar);
	return c->constants->count - 1;
}
