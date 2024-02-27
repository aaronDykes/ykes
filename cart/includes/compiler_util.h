#ifndef _COMPILER_UTIL_H
#define _COMPILER_UTIL_H
#include "scanner.h"
#include "chunk.h"

#define UINT8_COUNT \
    (UINT8_MAX + 1)

struct Parser
{
    token cur;
    token pre;
    bool err;
    bool panic;
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

typedef void (*parse_fn)();

struct parse_rule
{
    parse_fn prefix;
    parse_fn infix;
    Precedence prec;
};

struct Local
{
    arena name;
    int depth;
};
typedef struct Local Local;

struct Compiler
{
    Local locals[UINT8_COUNT];
    int local_count;
    int scope_depth;
};

typedef struct parse_rule PRule;
typedef struct Parser Parser;
typedef struct Compiler Compiler;

static Parser parser;
static Chunk compile_chunk;
static Compiler *current;

static void consume(int t, const char *str);
static void advance_compiler();

static void declaration();
static void var_dec();
static void synchronize();

static void statement();
static void print_statement();

static void begin_scope();
static void end_scope();
static void block();

static bool match(int t);
static bool check(int t);
static void expression();
static void grouping();
static PRule *get_rule(int t);
static void parse_precedence(Precedence precedence);

static void binary();
static void unary();

static void current_err(const char *err);
static void error(const char *err);
static void error_at(Token t, const char *err);

static void emit_byte(uint8_t byte);
static void emit_bytes(uint8_t b1, uint8_t b2);
static void emit_constant(arena ar);
static void emit_return();

static void dval();
static void ival();
static void llint();
static void ch();
static void boolean();
static void cstr();

static int resolve_local(Compiler *compiler, arena *name);

static arena parse_id();
static int parse_var(arena ar);
static void id();

static bool idcmp(arena a, arena b);
static void declare_var(arena ar);
static void add_local(arena *ar);

static PRule rules[] = {
    [TOKEN_CH_LPAREN] = {grouping, NULL, PREC_NONE},
    [TOKEN_CH_RPAREN] = {NULL, NULL, PREC_NONE},
    [TOKEN_CH_LCURL] = {NULL, NULL, PREC_NONE},
    [TOKEN_CH_RCURL] = {NULL, NULL, PREC_NONE},
    [TOKEN_CH_COMMA] = {NULL, NULL, PREC_NONE},
    [TOKEN_CH_SEMI] = {NULL, NULL, PREC_NONE},
    [TOKEN_CH_DOT] = {NULL, NULL, PREC_NONE},

    [TOKEN_OP_SUB] = {unary, binary, PREC_TERM},
    [TOKEN_OP_ADD] = {NULL, binary, PREC_TERM},

    [TOKEN_OP_DIV] = {NULL, binary, PREC_FACTOR},
    [TOKEN_OP_MOD] = {NULL, binary, PREC_FACTOR},
    [TOKEN_OP_MUL] = {NULL, binary, PREC_FACTOR},

    [TOKEN_OP_ASSIGN] = {NULL, NULL, PREC_ASSIGNMENT},
    [TOKEN_OP_BANG] = {unary, NULL, PREC_TERM},
    [TOKEN_OP_NE] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_OP_EQ] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_OP_GT] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_OP_GE] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_OP_LT] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_OP_LE] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_OP_AND] = {NULL, binary, PREC_AND},
    [TOKEN_OP_OR] = {NULL, binary, PREC_OR},

    [TOKEN_FALSE] = {boolean, NULL, PREC_NONE},
    [TOKEN_TRUE] = {boolean, NULL, PREC_NONE},

    [TOKEN_ID] = {id, NULL, PREC_NONE},
    [TOKEN_STR] = {cstr, NULL, PREC_NONE},
    [TOKEN_BTYE] = {NULL, NULL, PREC_NONE},

    [TOKEN_CHAR] = {ch, NULL, PREC_NONE},
    [TOKEN_INT] = {ival, NULL, PREC_NONE},
    [TOKEN_DOUBLE] = {dval, NULL, PREC_NONE},
    [TOKEN_LLINT] = {llint, NULL, PREC_NONE},
    [TOKEN_CLASS] = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
    [TOKEN_FOR] = {NULL, NULL, PREC_NONE},

    [TOKEN_LINE_COMMENT] = {unary, NULL, PREC_NONE},
    [TOKEN_NLINE_COMMENT] = {unary, NULL, PREC_NONE},

    [TOKEN_IF] = {NULL, NULL, PREC_NONE},
    [TOKEN_NULL] = {boolean, NULL, PREC_NONE},
    [TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PREC_NONE},
    [TOKEN_SUPER] = {NULL, NULL, PREC_NONE},
    [TOKEN_THIS] = {NULL, NULL, PREC_NONE},
    [TOKEN_VAR] = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PREC_NONE},
    [TOKEN_ERR] = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE},
};

static void end_compile();
static void init_compiler(Compiler *compiler);

#endif