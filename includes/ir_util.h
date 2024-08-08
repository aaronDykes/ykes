
#ifndef _COMPILER_UTIL_H
#define _COMPILER_UTIL_H
#include "parser.h"
#include "object.h"
#define LOCAL_COUNT 255

typedef struct compiler compiler;
typedef struct local local;
typedef struct counter counter;
typedef struct lookup lookup;
typedef struct state state;

typedef enum
{
    _FUNCTION,
    _SCRIPT
} compiler_t;

struct local
{
    key name;
    uint8_t scope;
};

struct counter
{
    uint8_t local;
    uint8_t scope;
    uint8_t obj;
    uint8_t include;
    uint8_t upvalue;
};

struct state
{
    compiler_t type;
    uint8_t flags;
    uint16_t line;
    counter count;
};

struct compiler
{
    state state;
    table *lookup;
    ast_stack *ast;

    function *func;
    local locals[255];
    compiler *base;
    compiler *prev;
};

static void init_compiler(compiler *c, compiler_t type, key name);

static void end_scope(compiler *c);
static void emit_id_resolution(compiler *compiler, key _key);
static void emit_loop(compiler *compiler, int index);
static void emit_byte(compiler *compiler, uint8_t op_code);
static void emit_bytes(compiler *compiler, uint8_t op_code, uint8_t arg);
static void emit_const_token(compiler *compiler, ast *s);
static void emit_const_element(compiler *compiler, element el);
static element create_const(ast *s);

static int emit_jmp(compiler *compiler, uint8_t op_code);
static void patch_jmp(compiler *compiler, int offset);

static void eval_expression(compiler *compiler, ast **ast);
static void eval_expr(compiler *compiler, ast **s);
static void eval_block(compiler *compiler, ast_stack *stk);
static void eval_statement(compiler *compiler, ast_stack *ast);
static void eval_ast(compiler *compiler);

static uint8_t id_cmp(key a, key b);
static int resolve_local(compiler *c, key name);
static int parse_var(compiler *c, key name);

static int resolve_func(compiler *compiler, Long name);

static void add_local(compiler *c, key local);

#endif
