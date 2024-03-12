#ifndef _STACK_H
#define _STACK_H

#include "arena_memory.h"

#define GROW_STACK(st, size) \
    realloc_stack(st, size)
#define FREE_STACK(st) \
    realloc_stack(st, 0)
#define FREE_FUNCTION(func) \
    free_function(func)
#define FREE_NATIVE(nat) \
    free_native(nat)
#define FREE_CLOSURE(clos) \
    free_closure(clos)
#define FREE_UPVAL(up) \
    free_upval(up)
#define NEW_STACK(size) \
    stack(size)
#define OBJ(o) \
    Obj(o)
#define FUNC(ar) \
    Func(ar)
#define NATIVE(n) \
    native_fn(n)
#define CLOSURE(c) \
    closure(c)
#define UPVAL(c) \
    up_val(c)

typedef enum
{
    OP_CONSTANT,
    OP_CLOSURE,
    OP_PRINT,

    OP_POP,
    OP_POPN,
    OP_PUSH,
    OP_CLOSE_UPVAL,

    OP_GLOBAL_DEF,

    OP_GET_GLOBAL,
    OP_SET_GLOBAL,

    OP_GET_LOCAL,
    OP_SET_LOCAL,

    OP_GET_UPVALUE,
    OP_SET_UPVALUE,

    OP_ASSIGN,
    OP_NEG,

    OP_INC_LOC,
    OP_INC_GLO,
    OP_DEC_LOC,
    OP_DEC_GLO,

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
    FN_CLOSURE,
    FUNCTION,
    NATIVE_FN,
    SCRIPT
} FT;

typedef enum
{
    ARENA,
    NATIVE,
    CLOSURE,
    FUNC,
} ObjType;

typedef struct Chunk Chunk;
typedef struct Function Function;
typedef struct Closure Closure;
typedef struct Upval Upval;
typedef struct Native Native;
typedef struct Element Element;
typedef struct Stack Stack;
typedef Element (*NativeFn)(int argc, Stack *argv);

struct Chunk
{

    int line;

    Arena cases;
    Arena op_codes;

    Stack *constants;
};

struct Function
{
    int arity;
    int upvalue_count;
    Arena name;
    Chunk ch;
};

struct Upval
{
    int len;
    int count;
    size_t size;
    Stack *index;
};

struct Closure
{
    Function *func;
    Upval *upvals;
    int upval_count;
};

struct Native
{
    int arity;
    Arena obj;
    NativeFn fn;
};

struct Element
{
    ObjType type;

    union
    {
        Arena arena;
        Function *func;
        Native *native;
        Closure closure;
    };
};

struct Stack
{

    int count;
    int len;
    size_t size;
    Element as;
    struct Stack *top;
};

void init_chunk(Chunk *c);
void write_chunk(Chunk *c, uint8_t byte);
int add_constant(Chunk *c, Element ar);
void free_chunk(Chunk *c);

Stack *stack(size_t size);
Stack *realloc_stack(Stack *stack, size_t size);
void free_stack(Stack **stack);

Upval *indices(size_t size);
void free_indices(Upval *up);

Element Obj(Arena ar);
Element Func(Function *f);
Element native_fn(Native *native);
Element closure(Closure clos);

Function *function();
void free_function(Function *func);

Closure new_closure(Function *func);
void free_closure(Closure *closure);

Upval upval(Stack *index);
void free_upval(Upval *up);

Native *native(NativeFn native, Arena ar);
void free_native(Native *native);

void print(Element ar);
void push(Stack **s, Element e);
void popn(Stack **s, int ival);
void check_stack_size(Stack *s);
void reset_stack(Stack *s);

#endif