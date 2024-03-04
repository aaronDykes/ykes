#ifndef _COMPILER_UTIL_H
#define _COMPILER_UTIL_H
#include "scanner.h"

#define MAX_ELIF 10
#define LOCAL_COUNT 500
#define PTR_SIZE(X) sizeof(X) / sizeof(X[0])

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

struct Local
{
    arena name;
    int depth;
};
typedef struct Local Local;

typedef struct parse_rule PRule;
typedef struct Parser Parser;

struct Compiler
{
    int local_count;
    int scope_depth;

    FT type;
    Function *func;

    struct Compiler *enclosing;
    Local locals[LOCAL_COUNT];
    Parser parser;
};

typedef struct Compiler Compiler;

typedef void (*parse_fn)(Compiler *);

struct parse_rule
{
    parse_fn prefix;
    parse_fn infix;
    Precedence prec;
};

static void consume(int t, const char *str, Parser *parser);
static void advance_compiler(Parser *parser);

static void declaration(Compiler *c);
static void call(Compiler *c);

static int argument_list(Compiler *c);

static void func_declaration(Compiler *c);
static void func_body(Compiler *c, FT type, arena ar);
static void func_var(Compiler *c);

static void var_dec(Compiler *c);
static void synchronize(Parser *parser);

static void statement(Compiler *c);
static void print_statement(Compiler *c);

static void begin_scope(Compiler *c);
static void end_scope(Compiler *c);

static void parse_block(Compiler *c);
static void block(Compiler *c);

static void comment(Compiler *c);

static void emit_loop(Compiler *c, int byte);
static int emit_jump_long(Chunk *ch, int byte);
static int emit_jump(Chunk *ch, int byte);

static void patch_jump_long(Compiler *c, int begin, int byte);
static void patch_jump(Compiler *c, int byte);

static void for_statement(Compiler *c);
static void while_statement(Compiler *c);

static void consume_if(Compiler *c);
static void consume_elif(Compiler *c);
static arena consume_switch(Compiler *c);

static void switch_statement(Compiler *c);
static void case_statement(Compiler *c, arena ar);

static void if_statement(Compiler *c);
static void elif_statement(Compiler *c);

static void default_expression(Compiler *c);
static void expression(Compiler *c);

static bool match(int t, Parser *parser);
static bool check(int t, Parser *parser);
static bool is_comment(Parser *parser);

static void grouping(Compiler *c);
static PRule *get_rule(int t);
static void parse_precedence(Precedence precedence, Compiler *c);

static void _and(Compiler *c);
static void _or(Compiler *c);

static void binary(Compiler *c);
static void unary(Compiler *c);

static void current_err(const char *err, Parser *parser);
static void error(const char *err, Parser *parser);
static void error_at(Token t, Parser *parser, const char *err);

static void emit_byte(Chunk *ch, uint8_t byte);
static void emit_bytes(Chunk *ch, uint8_t b1, uint8_t b2);
static void emit_constant(Chunk *ch, arena ar);
static void emit_return(Chunk *ch);

static void dval(Compiler *c);
static void ival(Compiler *c);
static void llint(Compiler *c);
static void ch(Compiler *c);
static void boolean(Compiler *c);
static void cstr(Compiler *c);

static int resolve_local(Compiler *c, arena *name);

static arena parse_func_id(Compiler *c);
static arena parse_id(Compiler *c);
static int parse_var(Compiler *c, arena ar);
static void id(Compiler *c);
static arena get_id(Compiler *c);

static bool idcmp(arena a, arena b);
static void declare_var(Compiler *c, arena ar);
static void add_local(Compiler *c, arena *ar);

static PRule rules[] = {
    [TOKEN_CH_LPAREN] = {grouping, call, PREC_NONE},
    [TOKEN_CH_RPAREN] = {NULL, NULL, PREC_NONE},
    [TOKEN_CH_LCURL] = {NULL, NULL, PREC_NONE},
    [TOKEN_CH_RCURL] = {NULL, NULL, PREC_NONE},
    [TOKEN_CH_COMMA] = {NULL, NULL, PREC_NONE},
    [TOKEN_CH_SEMI] = {NULL, NULL, PREC_NONE},
    [TOKEN_CH_DOT] = {NULL, NULL, PREC_NONE},

    [TOKEN_OP_INC] = {id, NULL, PREC_TERM},
    [TOKEN_OP_DEC] = {NULL, NULL, PREC_TERM},

    [TOKEN_OP_SUB] = {unary, binary, PREC_TERM},
    [TOKEN_OP_ADD] = {NULL, binary, PREC_TERM},

    [TOKEN_OP_DIV] = {NULL, binary, PREC_FACTOR},
    [TOKEN_OP_MOD] = {NULL, binary, PREC_FACTOR},
    [TOKEN_OP_MUL] = {NULL, binary, PREC_FACTOR},

    [TOKEN_OP_ASSIGN] = {NULL, NULL, PREC_ASSIGNMENT},
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
    [TOKEN_ELIF] = {NULL, NULL, PREC_OR},
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

static Function *end_compile(Compiler *compiler);
static void init_compiler(Compiler *a, Compiler *b, FT type);

#endif