#ifndef _STACK_H
#define _STACK_H

#include "arena_memory.h"

#define GROW_STACK(st, size) \
    realloc_stack(st, size)
#define FREE_STACK(st) \
    realloc_stack(st, 0)

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
    OP_INC,
    OP_DEC,
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

    OP_JMPL,
    OP_JMPC,
    OP_JMPF,
    OP_JMPT,
    OP_JMP,
    OP_LOOP,

    OP_CALL,

    OP_NOOP,
    OP_NULL,

    OP_RETURN

} opcode;

typedef enum
{
    FUNCTION,
    SCRIPT
} FT;

typedef enum
{
    ARENA,
    FUNC
} T;

typedef struct Stack Stack;
struct Chunk
{

    int line;

    arena cases;
    arena op_codes;

    Stack *constants;
};
typedef struct Chunk Chunk;

struct Function
{
    int arity;
    arena name;
    Chunk ch;
};
typedef struct Function Function;

struct Element
{
    T type;

    union
    {
        arena arena;
        Function *func;
    };
};

typedef struct Element Element;

struct Stack
{

    int count;
    int len;
    size_t size;
    Element as;
    struct Stack *top;
};

typedef struct Stack Stack;

void init_chunk(Chunk *c);
void write_chunk(Chunk *c, uint8_t byte);
int add_constant(Chunk *c, Element ar);
void free_chunk(Chunk *c);

Stack *stack(size_t size);
Stack *realloc_stack(Stack *stack, size_t size);
void free_stack(Stack *stack);

Element Obj(arena ar);
Element Func(Function *f);

void free_function(Function *func);
Function *function();

void print(Element ar);
void push(Stack **s, Element e);
void popn(Stack **s, int ival);
void check_stack_size(Stack *s);
void reset_stack(Stack *s);

#endif