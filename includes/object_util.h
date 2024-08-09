#ifndef _OBJECT_UTIL_H
#define _OBJECT_UTIL_H
#include <stdbool.h>
#include <stdlib.h>

#define CLOSURE(el) ((closure *)(el.as.Obj))
#define FUNC(el)    ((function *)(el.as.Obj))
#define NATIVE(el)  ((native *)(el.as.Obj))
#define STRUCT(el)  ((structure *)(el.as.Obj))
#define TABLE(el)   ((table *)(el.as.Obj))
#define STACK(el)   ((stack *)(el.as.Obj))
#define UPVAL(el)   ((upval *)(el.as.Obj))

typedef enum
{
	OP_CONSTANT,
	OP_CLOSURE,
	OP_PRINT,

	OP_STRUCT,
	OP_GET_INSTANCE,
	OP_ALLOC_TABLE,

	OP_POP,
	OP_POPN,
	OP_RM,
	OP_CLOSE_UPVAL,

	OP_GET_PROP,
	OP_SET_PROP,
	OP_SET_FIELD,
	OP_GET_FIELD,

	OP_GET_METHOD,
	OP_GET_OBJ,
	OP_SET_OBJ,

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

	OP_ADD_ASSIGN,
	OP_SUB_ASSIGN,
	OP_MUL_ASSIGN,
	OP_DIV_ASSIGN,
	OP_MOD_ASSIGN,
	OP_AND_ASSIGN,
	OP__OR_ASSIGN,

	OP_CAST,

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

	OP_EQ,
	OP_NE,
	OP_LT,
	OP_LE,
	OP_GT,
	OP_GE,

	OP_JMP_NIL,
	OP_JMP_NOT_NIL,
	OP_JMPL,
	OP_JMPL_T,
	OP_JMPL_F,
	OP_JMPF,
	OP_JMPT,
	OP_JMP,
	OP_LOOP,

	OP_CALL,
	OP_INSTANCE,
	OP_METHOD,
	OP_TO_STR,

	OP_NOOP,
	OP_RETURN,

} opcode_t;

typedef enum
{
	CAST_NUM_CHAR,
	CAST_NUM_STR,
	CAST_CHAR_NUM,
	CAST_CHAR_STR,
	CAST_BOOL_NUM,
	CAST_BOOL_STR,
	CAST_STR_NUM,
	CAST_STR_BOOL,
	CAST_STR_CHAR,
} cast_t;

typedef enum
{

	T_NUM,
	T_CHAR,
	T_STR,
	T_BOOL,

	T_KEY,
	T_NATIVE,
	T_STRUCT,
	T_CLOSURE,
	T_FUNCTION,
	T_VECTOR,
	T_INCLUDE,

	T_UPVAL,
	T_METHOD,
	T_STACK,
	T_TABLE,
	T_NULL
} obj_t;

typedef struct vector     vector;
typedef struct _2d_vector _2d_vector;
typedef struct _3d_vector _3d_vector;
typedef union value       value;
typedef struct chunk      chunk;
typedef struct function   function;
typedef struct closure    closure;
typedef struct upval      upval;
typedef struct jmp_vector jmp_vector;
typedef struct buffer     buffer;
typedef struct native     native;
typedef struct element    element;
typedef struct stack      stack;

typedef struct structure structure;
typedef struct table     table;
typedef struct record    record;
typedef element (*NativeFn)(int argc, element *argv);
typedef struct _key _key;

struct _key
{
	int   hash;
	char *Str;
};

union value
{

	struct
	{
		uint16_t len;
		char    *String;
	};

	double Num;
	char   Char;
	bool   Bool;
	void  *Obj;
	_key   Key;
};

struct vector
{
	uint16_t count;
	uint16_t len;
	obj_t    type;
	value   *of;
};
struct _2d_vector
{
	uint16_t count;
	uint16_t len;
	value  **of;
};
struct _3d_vector
{
	uint16_t count;
	uint16_t len;
	value ***of;
};

struct buffer
{
	char   *bytes;
	uint8_t count;
	uint8_t len;
};

struct jmp_vector
{
	uint16_t *bytes;
	uint16_t  count;
	uint16_t  len;
};

struct chunk
{
	uint16_t   count;
	uint16_t   len;
	uint8_t   *ip;
	uint16_t  *lines;
	jmp_vector cases;
	jmp_vector expr;
	stack     *constants;
};

struct function
{
	uint8_t arity;
	uint8_t uargc;
	uint8_t objc;
	_key    name;
	chunk   ch;
};

struct closure
{
	function *func;
	upval   **upvals;
	uint8_t   uargc;
};

struct native
{
	uint8_t  arity;
	_key     name;
	NativeFn fn;
};

struct element
{
	obj_t type;
	value as;
};

struct upval
{
	uint8_t index;
	element closed;
	upval  *next;
};
struct structure
{
	_key   name;
	table *fields;
};

struct stack
{
	uint16_t count;
	uint16_t len;
	element *as;
};

struct record
{
	_key    key;
	element val;
	record *next;
};

struct table
{
	uint16_t count;
	uint16_t len;
	record  *records;
};
#endif
