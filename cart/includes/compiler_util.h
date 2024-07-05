#ifndef _COMPILER_UTIL_H
#define _COMPILER_UTIL_H
#include "scanner.h"
#include "arena_memory.h"

#define MAX_ELIF 10
#define LOCAL_COUNT 255
#define CALL_COUNT 255
#define CLASS_COUNT 50
#define CWD_MAX 1024
#define _SIZE(X) sizeof(X) / sizeof(X[0])

#define INSTANCE_SET 0x01 /* 0000 0001 */
#define INSTANCE_CLR 0xFE /* 1111 1110 */

#define FLAG_INSTANCE(n) \
    (n & INSTANCE_SET)

struct parser
{
    token cur;
    token pre;
    bool err;
    bool panic;
    const char *current_file;
};

typedef enum
{
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_OR,         // or
    PREC_AND,        // and
    PREC_EQUALITY,   // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM,       // + -
    PREC_FACTOR,     // * /
    PREC_UNARY,      // ! -
    PREC_CALL,       // . ()
    PREC_PRIMARY

} Precedence;

typedef struct parser parser;
typedef struct local local;
typedef struct upvalue upvalue;
typedef struct compiler compiler;
typedef struct class_compiler class_compiler;
typedef void (*parse_fn)(compiler *);
typedef struct parse_rule PRule;
typedef struct counter counter;
typedef struct current current;
typedef struct hash_ref hash_ref;
typedef struct meta meta;
typedef struct lookup lookup;
typedef struct compiler_stack compiler_stack;

struct local
{
    arena name;
    int depth;
    bool captured;
};

struct upvalue
{
    uint8_t index;
    bool islocal;
};

struct class_compiler
{
    class_compiler *enclosing;
    arena instance_name;
};

struct counter
{
    uint8_t local;
    uint8_t scope;
    uint8_t upvalue;
    uint8_t call;
    uint8_t param;
    uint8_t class;
    uint8_t native;
};

struct current
{
    uint8_t index;
    uint8_t set;
    uint8_t get;
};

struct hash_ref
{
    arena init;
    arena len;
    arena push;
    arena pop;
};

struct meta
{
    uint8_t flags;
    ObjType type;
    const char *cwd;
};

struct lookup
{
    table *call;
    table *class;
    table *include;
    table *native;
};

struct compiler_stack
{
    class *instance[CALL_COUNT];
    local local[LOCAL_COUNT];
    upvalue upvalue[LOCAL_COUNT];
};

struct compiler
{
    counter count;
    current array;

    meta meta;
    hash_ref _hash_ref;
    lookup lookup;

    parser parser;
    function *func;

    compiler *base;
    compiler *enclosing;

    compiler_stack stack;
    class_compiler *class_compiler;
};

struct parse_rule
{
    parse_fn prefix;
    parse_fn infix;
    Precedence prec;
};

static void consume(int t, const char *str, parser *parser);
static void advance_compiler(parser *parser);

static void declaration(compiler *c);
static void call(compiler *c);

static int argument_list(compiler *c);

static void class_declaration(compiler *c);
static void method(compiler *c, class *class);
static void method_body(compiler *c, ObjType type, arena ar, class **class);

static void func_declaration(compiler *c);
static void func_body(compiler *c, ObjType type, arena ar);
static void func_var(compiler *c);

static void var_dec(compiler *c);
static void synchronize(parser *parser);

static void statement(compiler *c);
static void print_statement(compiler *c);

static void begin_scope(compiler *c);
static void end_scope(compiler *c);

static void parse_block(compiler *c);
static void block(compiler *c);

// static void comment(compiler *c);

static void emit_loop(compiler *c, int byte);
static int emit_jump_long(compiler *c, int byte);
static int emit_jump(compiler *c, int byte);

static void patch_jump_long(compiler *c, int begin, int byte);
static void patch_jump(compiler *c, int byte);

static void for_statement(compiler *c);
static void while_statement(compiler *c);
static void each_statement(compiler *c);

static void rm_statement(compiler *c);

static void consume_if(compiler *c);
static void consume_elif(compiler *c);
static arena consume_switch(compiler *c);

static void switch_statement(compiler *c);
static void case_statement(compiler *c, arena ar);

static void if_statement(compiler *c);
static void elif_statement(compiler *c);
static void ternary_statement(compiler *c);
static void null_coalescing_statement(compiler *c);

static void return_statement(compiler *c);

static void default_expression(compiler *c);
static void expression(compiler *c);

static bool match(int t, parser *parser);
static bool check(int t, parser *parser);
static bool is_comment(parser *parser);

static void grouping(compiler *c);
static PRule *get_rule(int t);
static void parse_precedence(Precedence precedence, compiler *c);

static void _and(compiler *c);
static void _or(compiler *c);

static void binary(compiler *c);
static void unary(compiler *c);

static void current_err(const char *err, parser *parser);
static void error(const char *err, parser *parser);
static void error_at(Token t, parser *parser, const char *err);

static void emit_byte(compiler *c, uint8_t byte);
static void emit_bytes(compiler *c, uint8_t b1, uint8_t b2);
static void emit_constant(compiler *c, arena ar);
static void emit_return(compiler *c);

static void array(compiler *c);
static void _access(compiler *c);
static void dval(compiler *c);
static void pi(compiler *c);

static void euler(compiler *c);

static void ival(compiler *c);
static void llint(compiler *c);
static void ch(compiler *c);
static void boolean(compiler *c);
static void cstr(compiler *c);

static const char *parse_string(compiler *c);
static void string(compiler *c);
static void array_alloc(compiler *c);
static void vector_alloc(compiler *c);
static void stack_alloc(compiler *c);

static void _table(compiler *c);

static int resolve_local(compiler *c, arena *name);
static int resolve_upvalue(compiler *c, arena *name);
static int add_upvalue(compiler *c, int upvalue, bool t);

static arena parse_func_id(compiler *c);
static void push_array_val(compiler *c);

static void parse_native_var_arg(compiler *c);

static void dot(compiler *c);
static void _this(compiler *c);

static arena parse_id(compiler *c);

static int parse_var(compiler *c, arena ar);
static void id(compiler *c);
static arena get_id(compiler *c);

static bool idcmp(arena a, arena b);
static void declare_var(compiler *c, arena ar);
static void add_local(compiler *c, arena *ar);

static PRule rules[] = {
    [TOKEN_CH_LPAREN] = {grouping, call, PREC_CALL},
    [TOKEN_CH_RPAREN] = {NULL, NULL, PREC_NONE},

    [TOKEN_CH_LCURL] = {NULL, NULL, PREC_NONE},
    [TOKEN_CH_RCURL] = {NULL, NULL, PREC_NONE},

    [TOKEN_CH_LSQUARE] = {array, _access, PREC_CALL},
    [TOKEN_CH_RSQUARE] = {NULL, NULL, PREC_NONE},

    [TOKEN_CH_COMMA] = {NULL, NULL, PREC_NONE},
    [TOKEN_CH_SEMI] = {NULL, NULL, PREC_NONE},
    [TOKEN_CH_DOT] = {NULL, dot, PREC_CALL},

    [TOKEN_OP_INC] = {id, NULL, PREC_TERM},
    [TOKEN_OP_DEC] = {NULL, NULL, PREC_TERM},

    [TOKEN_OP_SUB] = {unary, binary, PREC_TERM},
    [TOKEN_OP_ADD] = {NULL, binary, PREC_TERM},

    [TOKEN_OP_DIV] = {NULL, binary, PREC_FACTOR},
    [TOKEN_OP_MOD] = {NULL, binary, PREC_FACTOR},
    [TOKEN_OP_MUL] = {NULL, binary, PREC_FACTOR},

    [TOKEN_OP_ASSIGN] = {NULL, NULL, PREC_ASSIGNMENT},
    [TOKEN_ADD_ASSIGN] = {NULL, NULL, PREC_ASSIGNMENT},
    [TOKEN_SUB_ASSIGN] = {NULL, NULL, PREC_ASSIGNMENT},
    [TOKEN_MUL_ASSIGN] = {NULL, NULL, PREC_ASSIGNMENT},
    [TOKEN_DIV_ASSIGN] = {NULL, NULL, PREC_ASSIGNMENT},
    [TOKEN_MOD_ASSIGN] = {NULL, NULL, PREC_ASSIGNMENT},
    [TOKEN_AND_ASSIGN] = {NULL, NULL, PREC_ASSIGNMENT},
    [TOKEN_OR__ASSIGN] = {NULL, NULL, PREC_ASSIGNMENT},

    [TOKEN_OP_BANG] = {unary, NULL, PREC_TERM},
    [TOKEN_OP_SNE] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_OP_SEQ] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_OP_NE] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_OP_EQ] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_OP_GT] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_OP_GE] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_OP_LT] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_OP_LE] = {NULL, binary, PREC_COMPARISON},

    [TOKEN_SC_AND] = {NULL, _and, PREC_AND},
    [TOKEN_SC_OR] = {NULL, _or, PREC_OR},

    [TOKEN_OP_AND] = {NULL, binary, PREC_AND},
    [TOKEN_OP_OR] = {NULL, binary, PREC_OR},

    [TOKEN_FALSE] = {boolean, NULL, PREC_NONE},
    [TOKEN_TRUE] = {boolean, NULL, PREC_NONE},
    [TOKEN_EACH] = {NULL, NULL, PREC_NONE},

    [TOKEN_ID] = {id, NULL, PREC_NONE},
    [TOKEN_STR] = {cstr, NULL, PREC_NONE},
    [TOKEN_ALLOC_STR] = {string, NULL, PREC_NONE},
    [TOKEN_ALLOC_ARRAY] = {array_alloc, NULL, PREC_NONE},
    [TOKEN_ALLOC_VECTOR] = {vector_alloc, NULL, PREC_NONE},
    [TOKEN_ALLOC_STACK] = {stack_alloc, NULL, PREC_NONE},
    [TOKEN_TABLE] = {_table, NULL, PREC_NONE},
    [TOKEN_CH_TERNARY] = {NULL, NULL, PREC_NONE},
    [TOKEN_CH_NULL_COALESCING] = {NULL, NULL, PREC_NONE},

    [TOKEN_CHAR] = {ch, NULL, PREC_NONE},
    [TOKEN_INT] = {ival, NULL, PREC_NONE},
    [TOKEN_DOUBLE] = {dval, NULL, PREC_NONE},
    [TOKEN_LLINT] = {llint, NULL, PREC_NONE},
    [TOKEN_CLASS] = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
    [TOKEN_FOR] = {NULL, NULL, PREC_NONE},

    [TOKEN_LINE_COMMENT] = {NULL, NULL, PREC_NONE},
    [TOKEN_NLINE_COMMENT] = {NULL, NULL, PREC_NONE},

    [TOKEN_IF] = {NULL, NULL, PREC_NONE},
    [TOKEN_ELIF] = {NULL, NULL, PREC_OR},
    [TOKEN_NULL] = {boolean, NULL, PREC_NONE},

    [TOKEN_CLOCK] = {parse_native_var_arg, NULL, PREC_CALL},
    [TOKEN_SQRT] = {parse_native_var_arg, NULL, PREC_CALL},
    [TOKEN_PRIME] = {parse_native_var_arg, NULL, PREC_CALL},
    [TOKEN_FILE] = {parse_native_var_arg, NULL, PREC_CALL},

    [TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PREC_NONE},
    [TOKEN_SUPER] = {NULL, NULL, PREC_NONE},
    [TOKEN_THIS] = {_this, NULL, PREC_NONE},
    [TOKEN_VAR] = {NULL, NULL, PREC_NONE},
    [TOKEN_TYPE_ARRAY] = {NULL, NULL, PREC_NONE},
    [TOKEN_TYPE_INT] = {NULL, NULL, PREC_NONE},
    [TOKEN_TYPE_DOUBLE] = {NULL, NULL, PREC_NONE},
    [TOKEN_TYPE_LONG] = {NULL, NULL, PREC_NONE},
    [TOKEN_TYPE_STRING] = {NULL, NULL, PREC_NONE},
    [TOKEN_TYPE_CHAR] = {NULL, NULL, PREC_NONE},
    [TOKEN_TYPE_BYTE] = {NULL, NULL, PREC_NONE},

    [TOKEN_PI] = {pi, NULL, PREC_NONE},
    [TOKEN_EULER] = {euler, NULL, PREC_NONE},

    [TOKEN_WHILE] = {NULL, NULL, PREC_NONE},
    [TOKEN_ERR] = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE},
};

static function *end_compile(compiler *a);
static void init_compiler(compiler *a, compiler *b, ObjType type, arena ar);

#endif
