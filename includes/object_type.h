#ifndef _OBJECT_TYPE_H
#define _OBJECT_TYPE_H

typedef struct vector         vector;
typedef struct _2d_vector     _2d_vector;
typedef struct _3d_vector     _3d_vector;
typedef union value           value;
typedef struct chunk          chunk;
typedef struct function       function;
typedef struct closure        closure;
typedef struct upval          upval;
typedef struct generic_vector generic_vector;
typedef struct buffer         buffer;
typedef struct native         native;
typedef struct element        element;
typedef struct stack          stack;
typedef struct init_table     init_table;
typedef struct field_stack    field_stack;

typedef enum opcode_t opcode_t;
typedef enum cast_t   cast_t;
typedef enum obj_t    obj_t;
typedef struct class class;
typedef struct table    table;
typedef struct record   record;
typedef struct instance instance;
typedef element (*NativeFn)(int argc, element *argv);
typedef struct _key _key;

#endif
