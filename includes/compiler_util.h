#ifndef _COMPILER_UTIL_H
#define _COMPILER_UTIL_H
#include "object_memory.h"
#include "scanner.h"

#define LOCAL_COUNT 255
#define CALL_COUNT  255
#define CLASS_COUNT 50
#define CWD_MAX     512

#define INSTANCE_SET 0x01 /* 0000 0001 */
#define INSTANCE_CLR 0xFE /* 1111 1110 */

#define FLAG_INSTANCE(n) (n & INSTANCE_SET)

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

} prec_t;

typedef enum
{
	COMPILER_TYPE_SCRIPT,
	COMPILER_TYPE_FUNCTION,
	COMPILER_TYPE_INIT,
	COMPILER_TYPE_METHOD,

} compiler_t;

typedef struct parser         parser;
typedef struct local          local;
typedef struct upvalue        upvalue;
typedef struct compiler       compiler;
typedef struct class_compiler class_compiler;
typedef void (*parse_fn)(compiler *);
typedef struct parse_rule     PRule;
typedef struct counter        counter;
typedef struct current        current;
typedef struct hash_ref       hash_ref;
typedef struct meta           meta;
typedef struct lookup         lookup;
typedef struct compiler_stack compiler_stack;

struct parser
{
	token       cur;
	token       pre;
	uint8_t     flag;
	const char *current_file;
};

struct local
{
	_key    name;
	uint8_t depth;
	bool    captured;
};

struct upvalue
{
	uint8_t index;
	bool    islocal;
};

struct class_compiler
{
	class_compiler *enclosing;
	uint8_t         index;
	_key            name;
};

struct counter
{
	uint8_t local;
	uint8_t scope;
	uint8_t upvalue;
	uint8_t obj;
};

struct current
{
	uint8_t index;
	uint8_t set;
	uint8_t get;
};

struct hash_ref
{
	int init;
	int len;
	int push;
	int pop;
};

struct meta
{
	uint8_t     flags;
	compiler_t  type;
	const char *cwd;
};

struct compiler_stack
{
	class *class[CALL_COUNT];
	local   local[LOCAL_COUNT];
	upvalue upvalue[LOCAL_COUNT];
};

struct compiler
{
	counter count;
	current array;

	meta     meta;
	hash_ref hash;
	table   *lookup;

	parser    parser;
	function *func;

	compiler *base;
	compiler *enclosing;

	compiler_stack  stack;
	class_compiler *class_compiler;
};

struct parse_rule
{
	parse_fn prefix;
	parse_fn infix;
	prec_t   prec;
};

static void consume(token_t t, const char *str, parser *parser);
static void advance_compiler(parser *parser);

static void declaration(compiler *c);
static void call(compiler *c);

static int argument_list(compiler *c);

static void class_declaration(compiler *c);
static void method(compiler *c, class *class);
static void method_body(compiler *c, compiler_t type, _key ar, class **class);

static void func_declaration(compiler *c);
static void func_body(compiler *c, _key ar);
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
static int  emit_jump(compiler *c, int byte);
static void patch_jump(compiler *c, int byte);

static void for_statement(compiler *c);
static void while_statement(compiler *c);

static void rm_statement(compiler *c);

static void consume_if(compiler *c);
static void consume_elif(compiler *c);
static void consume_switch(compiler *c);

static void switch_statement(compiler *c);
static void case_statement(compiler *c);

static void if_statement(compiler *c);
static void elif_statement(compiler *c);
static void ternary_statement(compiler *c);
static void null_coalescing_statement(compiler *c);

static void return_statement(compiler *c);

static void default_expression(compiler *c);
static void expression(compiler *c);

static bool match(token_t t, parser *parser);
static bool check(token_t t, parser *parser);
static bool is_comment(parser *parser);

static void   grouping(compiler *c);
static PRule *get_rule(int t);
static void   parse_precedence(prec_t precedence, compiler *c);

static void _and(compiler *c);
static void _or(compiler *c);

static void cast(compiler *c);

static void binary(compiler *c);
static void unary(compiler *c);
static void infix_unary(compiler *c);
static void compound_assign(compiler *c);

static void current_err(const char *err, parser *parser);
static void prev_error(const char *err, parser *parser);
static void error_at(Token t, parser *parser, const char *err);

static void emit_byte(compiler *c, uint8_t byte);
static void emit_bytes(compiler *c, uint8_t b1, uint8_t b2);
static void emit_constant(compiler *c, element ar);
static void emit_return(compiler *c);

static void pi(compiler *c);
static void euler(compiler *c);
static void num(compiler *c);
static void ch(compiler *c);
static void boolean(compiler *c);
static void str(compiler *c);
static void fmt_str(compiler *c);

static _key parse_string(compiler *c);
static void stack_alloc(compiler *c);

static void _table(compiler *c);

static int resolve_local(compiler *c, _key *name);
static int resolve_upvalue(compiler *c, _key *name);
static int add_upvalue(compiler *c, int upvalue, bool t);

static void parse_native_var_arg(compiler *c);

static void dot(compiler *c);
static void _this(compiler *c);

static _key parse_id(compiler *c);

static int  parse_var(compiler *c, _key ar);
static void id(compiler *c);

static bool idcmp(_key a, _key b);
static void declare_var(compiler *c, _key ar);
static void add_local(compiler *c, _key *ar);

static PRule rules[] = {
    [TOKEN_CH_LPAREN]          = {grouping,             call,                      PREC_CALL      },
    [TOKEN_CH_RPAREN]          = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_CH_LCURL]           = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_CH_RCURL]           = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_CH_RSQUARE]         = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_CH_COMMA]           = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_CH_SEMI]            = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_CH_DOT]             = {NULL,                 dot,                       PREC_CALL      },
    [TOKEN_OP_INC]             = {unary,                infix_unary,               PREC_TERM      },
    [TOKEN_OP_DEC]             = {unary,                infix_unary,               PREC_TERM      },
    [TOKEN_OP_SUB]             = {unary,                binary,                    PREC_TERM      },
    [TOKEN_OP_ADD]             = {NULL,                 binary,                    PREC_TERM      },
    [TOKEN_OP_DIV]             = {NULL,                 binary,                    PREC_FACTOR    },
    [TOKEN_OP_MOD]             = {NULL,                 binary,                    PREC_FACTOR    },
    [TOKEN_OP_MUL]             = {NULL,                 binary,                    PREC_FACTOR    },
    [TOKEN_OP_ASSIGN]          = {NULL,                 NULL,                      PREC_ASSIGNMENT},
    [TOKEN_ADD_ASSIGN]         = {NULL,                 compound_assign,           PREC_ASSIGNMENT},
    [TOKEN_SUB_ASSIGN]         = {NULL,                 compound_assign,           PREC_ASSIGNMENT},
    [TOKEN_MUL_ASSIGN]         = {NULL,                 compound_assign,           PREC_ASSIGNMENT},
    [TOKEN_DIV_ASSIGN]         = {NULL,                 compound_assign,           PREC_ASSIGNMENT},
    [TOKEN_MOD_ASSIGN]         = {NULL,                 compound_assign,           PREC_ASSIGNMENT},
    [TOKEN_AND_ASSIGN]         = {NULL,                 compound_assign,           PREC_ASSIGNMENT},
    [TOKEN_OR__ASSIGN]         = {NULL,                 compound_assign,           PREC_ASSIGNMENT},
    [TOKEN_OP_BANG]            = {unary,                NULL,                      PREC_TERM      },
    [TOKEN_OP_NE]              = {NULL,                 binary,                    PREC_EQUALITY  },
    [TOKEN_OP_EQ]              = {NULL,                 binary,                    PREC_EQUALITY  },
    [TOKEN_OP_GT]              = {NULL,                 binary,                    PREC_COMPARISON},
    [TOKEN_OP_GE]              = {NULL,                 binary,                    PREC_COMPARISON},
    [TOKEN_OP_LT]              = {NULL,                 binary,                    PREC_COMPARISON},
    [TOKEN_OP_LE]              = {NULL,                 binary,                    PREC_COMPARISON},
    [TOKEN_SC_AND]             = {NULL,                 _and,                      PREC_AND       },
    [TOKEN_SC_OR]              = {NULL,                 _or,                       PREC_OR        },
    [TOKEN_OP_AND]             = {NULL,                 binary,                    PREC_AND       },
    [TOKEN_OP_OR]              = {NULL,                 binary,                    PREC_OR        },
    [TOKEN_FALSE]              = {boolean,              NULL,                      PREC_NONE      },
    [TOKEN_TRUE]               = {boolean,              NULL,                      PREC_NONE      },
    [TOKEN_EACH]               = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_ID]                 = {id,                   NULL,                      PREC_NONE      },
    [TOKEN_STR]                = {str,                  NULL,                      PREC_NONE      },
    [TOKEN_FMT_STR]            = {fmt_str,              NULL,                      PREC_NONE      },
    [TOKEN_ALLOC_STACK]        = {stack_alloc,          NULL,                      PREC_NONE      },
    [TOKEN_TABLE]              = {_table,               NULL,                      PREC_NONE      },
    [TOKEN_CH_TERNARY]         = {NULL,                 ternary_statement,         PREC_NONE      },
    [TOKEN_CH_NULL_COALESCING] = {NULL,                 null_coalescing_statement, PREC_NONE      },
    [TOKEN_CHAR]               = {ch,                   NULL,                      PREC_NONE      },
    [TOKEN_NUMBER]             = {num,                  NULL,                      PREC_NONE      },
    [TOKEN_CLASS]              = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_ELSE]               = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_FOR]                = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_LINE_COMMENT]       = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_NLINE_COMMENT]      = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_IF]                 = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_ELIF]               = {NULL,                 NULL,                      PREC_OR        },
    [TOKEN_NULL]               = {boolean,              NULL,                      PREC_NONE      },
    [TOKEN_CLOCK]              = {parse_native_var_arg, NULL,                      PREC_CALL      },
    [TOKEN_SQRT]               = {parse_native_var_arg, NULL,                      PREC_CALL      },
    [TOKEN_PRIME]              = {parse_native_var_arg, NULL,                      PREC_CALL      },
    [TOKEN_FILE]               = {parse_native_var_arg, NULL,                      PREC_CALL      },
    [TOKEN_ALLOC_STR]          = {parse_native_var_arg, NULL,                      PREC_NONE      },
    [TOKEN_PRINT]              = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_RETURN]             = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_SUPER]              = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_THIS]               = {_this,                NULL,                      PREC_NONE      },
    [TOKEN_VAR]                = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_TYPE_ARRAY]         = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_STORAGE_TYPE_NUM]   = {cast,                 NULL,                      PREC_CALL      },
    [TOKEN_STORAGE_TYPE_STR]   = {cast,                 NULL,                      PREC_CALL      },
    [TOKEN_STORAGE_TYPE_CHAR]  = {cast,                 NULL,                      PREC_CALL      },
    [TOKEN_STORAGE_TYPE_BOOL]  = {cast,                 NULL,                      PREC_CALL      },
    [TOKEN_PI]                 = {pi,                   NULL,                      PREC_NONE      },
    [TOKEN_EULER]              = {euler,                NULL,                      PREC_NONE      },
    [TOKEN_WHILE]              = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_ERR]                = {NULL,                 NULL,                      PREC_NONE      },
    [TOKEN_EOF]                = {NULL,                 NULL,                      PREC_NONE      },
};

static function *end_compile(compiler *a);
static void init_compiler(compiler *a, compiler *b, compiler_t type, _key ar);

#endif
