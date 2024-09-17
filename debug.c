#include "debug.h"
#include <stdio.h>

static int byte_instruction(const char *name, chunk *chunk, int offset)
{
	uint8_t slot = chunk->ip[offset + 1];
	printf("%-16s %4d\n", name, slot);
	return offset + 2;
}

static int simple_instruction(const char *name, int offset)
{
	printf("%s\n", name);
	return ++offset;
}

static int constant_instruction(const char *name, chunk *c, int offset)
{
	uint8_t constant = c->ip[offset + 1];

	printf("%-16s %4d '", name, constant);
	print(c->constants->as[constant]);
	printf("\n");
	return offset + 2;
}

static int
jump_instruction(const char *name, int sign, chunk *chunk, int offset)
{
	uint16_t jump =
	    (uint16_t)((chunk->ip[offset + 1] << 8) | (chunk->ip[offset + 2]));

	printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);

	return offset + 3;
}

void disassemble_chunk(chunk *c, const char *name)
{

	printf("==== chunk: `%s` ====\n", name);

	for (int i = 0; i < c->len;)
		i = disassemble_instruction(c, i);
}

int disassemble_instruction(chunk *c, int offset)
{

	printf("%d: %04d ", c->lines[offset], offset);

	switch (c->ip[offset])
	{
	case OP_CONSTANT:
		return constant_instruction("OP_CONSTANT", c, offset);
	case OP_CLOSURE:
	{
		offset++;
		uint8_t constant = c->ip[offset++];
		printf("%-16s %4d ", "OP_CLOSURE", constant);
		print(c->constants->as[constant]);

		closure *clos = CLOSURE(c->constants->as[constant]);
		if (!clos)
			return offset;

		for (int j = 0; j < clos->uargc; j++)
		{
			int isLocal = c->ip[offset++];
			int index   = c->ip[offset++];
			printf(
			    "%04d      |                     %s %d\n", offset - 2,
			    isLocal ? "local" : "upvalue", index
			);
		}

		return offset;
	}
	case OP_ALLOC_INSTANCE:
		return byte_instruction("OP_ALLOC_INSTANCE", c, offset);
	case OP_INSTANCE:
		return byte_instruction("OP_INSTANCE", c, offset);
	case OP_GET_OBJ:
		return byte_instruction("OP_GET_OBJ", c, offset);
	case OP_SET_OBJ:
		return byte_instruction("OP_SET_OBJ", c, offset);
	case OP_RESET_ARGC:
		return simple_instruction("OP_RESET_ARGC", offset);
	case OP_METHOD:
		return constant_instruction("OP_METHOD", c, offset);
	case OP_CLASS:
		return constant_instruction("OP_CLASS", c, offset);
	case OP_GET_INSTANCE:
		return constant_instruction("OP_GET_INSTANCE", c, offset);
	case OP_CLOSE_UPVAL:
		return simple_instruction("OP_CLOSE_UPVAL", offset);
	case OP_GET_UPVALUE:
		return byte_instruction("OP_GET_UPVALUE", c, offset);
	case OP_SET_UPVALUE:
		return byte_instruction("OP_SET_UPVALUE", c, offset);

	case OP_GET_METHOD:
		return simple_instruction("OP_GET_METHOD", offset);
	case OP_NEG:
		return simple_instruction("OP_NEG", offset);
	case OP_INC:
		return simple_instruction("OP_INC", offset);
	case OP_DEC:
		return simple_instruction("OP_DEC", offset);
	case OP_ADD:
		return simple_instruction("OP_ADD", offset);
	case OP_SUB:
		return simple_instruction("OP_SUB", offset);
	case OP_MUL:
		return simple_instruction("OP_MUL", offset);
	case OP_DIV:
		return simple_instruction("OP_DIV", offset);
	case OP_MOD:
		return simple_instruction("OP_MOD", offset);
	case OP_LT:
		return simple_instruction("OP_LT", offset);
	case OP_LE:
		return simple_instruction("OP_LE", offset);
	case OP_GT:
		return simple_instruction("OP_GT", offset);
	case OP_GE:
		return simple_instruction("OP_GE", offset);
	case OP_EQ:
		return simple_instruction("OP_EQ", offset);
	case OP_NE:
		return simple_instruction("OP_NE", offset);
	case OP_AND:
		return simple_instruction("OP_AND", offset);
	case OP_OR:
		return simple_instruction("OP_OR", offset);
	case OP_ALLOC_TABLE:
		return simple_instruction("OP_ALLOC_TABLE", offset);
	case OP_NOOP:
		return simple_instruction("OP_NOOP", offset);

	case OP_GET_LOCAL:
		return byte_instruction("OP_GET_LOCAL", c, offset);

	case OP_SET_LOCAL:
		return byte_instruction("OP_SET_LOCAL", c, offset);
	case OP_SET_LOCAL_PARAM:
		return byte_instruction("OP_SET_LOCAL_PARAM", c, offset);
	case OP_ALLOC_VECTOR:
		return simple_instruction("OP_ALLOC_VECTOR", offset);
	case OP_INIT_VECTOR:
		return byte_instruction("OP_INIT_VECTOR", c, offset);
	case OP_RM:
		return byte_instruction("OP_RM", c, offset);
	case OP_GET_GLOBAL:
		return simple_instruction("OP_GET_GLOBAL", offset);
	case OP_SET_GLOBAL:
		return simple_instruction("OP_SET_GLOBAL", offset);
	case OP_GLOBAL_DEF:
		return simple_instruction("OP_GLOBAL_DEF", offset);

	case OP_JMP_NIL:
		return jump_instruction("OP_JMP_NIL", 1, c, offset);
	case OP_JMP_NOT_NIL:
		return jump_instruction("OP_JMP_NOT_NIL", 1, c, offset);
	case OP_JMPF:
		return jump_instruction("OP_JMPF", 1, c, offset);
	case OP_JMPT:
		return jump_instruction("OP_JMPT", 1, c, offset);
	case OP_JMPL:
		return jump_instruction("OP_JMPL", 1, c, offset);
	case OP_JMP:
		return jump_instruction("OP_JMP", 1, c, offset);
	case OP_LOOP:
		return jump_instruction("OP_LOOP", -1, c, offset);
	case OP_POPN:
		return simple_instruction("OP_POPN", offset);
	case OP_POP:
		return simple_instruction("OP_POP", offset);
	case OP_CALL:
		return byte_instruction("OP_CALL", c, offset);
	case OP_CAST:
		return byte_instruction("OP_CAST", c, offset);
	case OP_TO_STR:
		return byte_instruction("OP_TO_STR", c, offset);
	case OP_PRINT:
		return simple_instruction("OP_PRINT", offset);
	case OP_RETURN:
		return simple_instruction("OP_RETURN", offset);
	case OP_GET_PROP:
		return byte_instruction("OP_GET_PROP", c, offset);
	case OP_SET_PROP:
		return byte_instruction("OP_SET_PROP", c, offset);

	case OP_THIS:
		return simple_instruction("OP_THIS", offset);
	case OP_GET_ACCESS:
		return simple_instruction("OP_GET_ACCESS", offset);
	case OP_SET_ACCESS:
		return simple_instruction("OP_SET_ACCESS", offset);

	case OP_DELETE_VAL:
		return simple_instruction("OP_DELETE_VAL", offset);
	case OP_INSERT_VAL:
		return simple_instruction("OP_INSERT_VAL", offset);
	case OP_LEN:
		return simple_instruction("OP_LEN", offset);

	default:
		printf("Unkown opcode: %d\n", offset);
		return ++offset;
	}
}
