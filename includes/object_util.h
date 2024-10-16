#ifndef _OBJECT_UTIL_H
#define _OBJECT_UTIL_H
#include <object_type.h>
#include <opcode.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define KEY(el)        ((_key *)el.obj)
#define CLOSURE(el)    ((closure *)(el.obj))
#define FUNC(el)       ((function *)(el.obj))
#define NATIVE(el)     ((native *)(el.obj))
#define CLASS(el)      ((class *)(el.obj))
#define INSTANCE(el)   ((instance *)(el.obj))
#define VECTOR(el)     ((vector *)(el.obj))
#define _2D_VECTOR(el) ((_2d_vector *)(el.obj))
#define _3D_VECTOR(el) ((_3d_vector *)(el.obj))
#define TABLE(el)      ((table *)(el.obj))
#define STACK(el)      ((stack *)(el.obj))
#define UPVAL(el)      ((upval *)(el.obj))
#define STR(el)        ((_string *)el.obj)

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
	T_CLASS,
	T_INSTANCE,
	T_CLOSURE,
	T_FUNCTION,
	T_VECTOR,
	T_VECTOR_2D,
	T_VECTOR_3D,
	T_INCLUDE,
	T_GEN,

	T_UPVAL,
	T_METHOD,
	T_STACK,
	T_TABLE,
	T_NULL
} obj_t;

struct _key
{
	int   hash;
	char *val;
};

struct _string
{
	int   len;
	char *String;
};

union value
{
	double Num;
	char   Char;
	bool   Bool;
};

struct vector
{
	int    count;
	int    len;
	obj_t  type;
	value *of;
};
struct _2d_vector
{
	int      count;
	int      len;
	obj_t    type;
	vector **of;
};
struct _3d_vector
{
	int          count;
	int          len;
	obj_t        type;
	_2d_vector **of;
};

struct buffer
{
	char *bytes;
	int   count;
	int   len;
};

struct generic_vector
{
	uint16_t *bytes;
	uint16_t  count;
	uint16_t  len;
};

struct chunk
{
	int            count;
	int            len;
	uint8_t       *ip;
	uint16_t      *lines;
	generic_vector cases;
	stack         *constants;
};

struct function
{
	uint8_t arity;
	uint8_t uargc;
	uint8_t objc;
	_key   *name;
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
	_key    *name;
	NativeFn fn;
};

struct element
{
	obj_t type;

	union
	{
		value val;
		void *obj;
	};
};

struct upval
{
	uint8_t index;
	element closed;
	upval  *next;
};
struct class
{
	closure *init;
	_key    *name;
	table   *closures;
};

struct instance
{
	class *classc;
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
	_key   *key;
	element val;
	record *next;
};

struct table
{
	uint16_t count;
	uint16_t len;
	record  *records;
};

struct init_table
{
	uint8_t init;
	table  *fields;
};

struct field_stack
{
	uint8_t     count;
	uint8_t     len;
	init_table *fields;
};

#endif
