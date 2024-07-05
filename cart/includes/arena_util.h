#ifndef _ARENA_UTIL_H
#define _ARENA_UTIL_H
#include <stdlib.h>
#include <stdbool.h>

#define OBJ(o) \
    Obj(o)
#define VECT(o) \
    vector_el(o)
#define FUNC(ar) \
    Func(ar)
#define NATIVE(n) \
    native_fn(n)
#define CLOSURE(c) \
    closure_el(c)
#define CLASS(c) \
    new_class(c)
#define INSTANCE(c) \
    new_instance(c)
#define TABLE(t) \
    table_el(t)

#define STK(stk) \
    stack_el(stk)

typedef enum
{

    ARENA_BYTE,
    ARENA_SIZE,
    ARENA_INT,
    ARENA_DOUBLE,
    ARENA_LONG,
    ARENA_CHAR,
    ARENA_STR,
    ARENA_CSTR,
    ARENA_BOOL,
    ARENA_NULL,
    ARENA_BYTES,
    ARENA_INTS,
    ARENA_DOUBLES,
    ARENA_LONGS,
    ARENA_BOOLS,
    ARENA_SIZES,
    ARENA_STRS,
    ARENA_VAR,

} T;

typedef enum
{
    OP_CONSTANT,
    OP_CLOSURE,
    OP_PRINT,

    OP_CLASS,
    OP_GET_INSTANCE,

    OP_EACH_ACCESS,
    OP_GET_ACCESS,
    OP_SET_ACCESS,
    OP_LEN,

    OP_ALLOC_TABLE,

    OP_PUSH_ARRAY_VAL,
    OP_POP__ARRAY_VAL,
    OP_PREPEND_ARRAY_VAL,

    OP_CPY_ARRAY,
    OP_POP,
    OP_POPN,
    OP_PUSH,
    OP_RM,
    OP_CLOSE_UPVAL,

    OP_PUSH_TOP,
    OP_GET_PROP,
    OP_SET_PROP,
    OP_SET_FIELD,
    OP_GET_FIELD,

    OP_FIND_CLOSURE,
    OP_GET_CLOSURE,
    OP_GET_METHOD,
    OP_GET_CLASS,
    OP_ALLOC_INSTANCE,
    OP_GET_NATIVE,

    OP_GLOBAL_DEF,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,

    OP_SET_FUNC_VAR,

    OP_RESET_ARGC,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_SET_LOCAL_PARAM,

    OP_GET_UPVALUE,
    OP_SET_UPVALUE,

    OP_ASSIGN,
    OP_ADD_ASSIGN,
    OP_SUB_ASSIGN,
    OP_MUL_ASSIGN,
    OP_DIV_ASSIGN,
    OP_MOD_ASSIGN,
    OP_AND_ASSIGN,
    OP__OR_ASSIGN,

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

    OP_JMP_NIL,
    OP_JMP_NOT_NIL,
    OP_JMPL,
    OP_JMPC,
    OP_JMPF,
    OP_JMPT,
    OP_JMP,
    OP_LOOP,

    OP_CALL,
    OP_METHOD,

    OP_NULL,

    OP_RETURN

} opcode;

typedef enum
{
    ARENA,
    NATIVE,
    CLASS,
    INSTANCE,
    CLOSURE,
    FUNCTION,
    VECTOR,

    METHOD,
    STACK,
    TABLE,
    INIT,
    UPVAL,
    SCRIPT,
    NULL_OBJ
} ObjType;

typedef enum
{
    OBJECT_TYPE,
    ARENA_TYPE,
    WILD_CARD
} Typed;

typedef union vector vector;
typedef struct value value;
typedef struct arena arena;
typedef struct data data;

typedef struct chunk chunk;
typedef struct function function;
typedef struct closure closure;
typedef struct upval upval;
typedef struct native native;
typedef struct element element;
typedef struct stack stack;
typedef struct class class;
typedef struct instance instance;
typedef struct table table;
typedef element (*NativeFn)(int argc, stack *argv);

struct _key
{
    long long int hash;
    char *key;
};

union vector
{
    uint8_t *Bytes;
    int *Ints;
    double *Doubles;
    long long int *Longs;
    char **Strings;
    bool *Bools;
    void *Void;
    size_t Sizes;
};

struct value
{

    long long int hash;
    union
    {
        struct
        {
            int len;
            int count;
            char *String;
        };

        size_t Size;
        uint8_t Byte;
        int Int;
        double Double;
        long long int Long;
        char Char;
        bool Bool;
        void *Void;
    };
};

struct arena
{
    size_t size;
    T type;

    union
    {
        struct
        {
            int count;
            int len;
            vector listof;
        };

        value as;
    };
};

struct chunk
{
    arena cases;
    arena op_codes;
    arena lines;
    stack *constants;
};

struct function
{
    int arity;
    int upvalue_count;
    arena name;
    chunk ch;
};

struct closure
{
    function *func;
    upval **upvals;
    int upval_count;
};

struct native
{
    int arity;
    arena obj;
    NativeFn fn;
};

struct element
{
    ObjType type;

    union
    {
        arena _arena;
        arena *_vector;
        native *native;
        closure *closure;
        class *classc;
        instance *instance;
        table *table;
        stack *stack;
        void *null;
    };
};

struct class
{
    closure *init;
    arena name;
    table *closures;
};

struct instance
{
    class *classc;
    table *fields;
};

struct stack
{
    int count;
    int len;
    size_t size;
    element as;
    stack *top;
};

struct upval
{
    int len;
    int count;
    size_t size;
    stack *index;
    stack closed;
    upval *next;
};

struct table
{
    size_t size;
    arena key;
    ObjType type;
    int count;
    int len;

    element val;
    table *next;
    table *prev;
};
#endif
