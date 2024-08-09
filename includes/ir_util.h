
#ifndef _IR_UTIL_H
#define _IR_UTIL_H
#include "ir_common.h"
#include "object.h"

static void    init_ir(ir *c);
static element create_const(ast *s);
static void    eval_expr(ir *ir, ast **s);
static void    eval_block(ir *ir, ast_stack **stk);
static void    eval_statement(ir *ir, ast_stack **ast);
static void    eval_ast(ir *ir);
static int     resolve_func(ir *ir, _key *name);

#endif

/*
static void consume(token_t t, const char *str, parser *parser);
static void advance_ir(parser *parser);

static void declaration(ir *c);
static void call(ir *c);

static int argument_list(ir *c);

static void class_declaration(ir *c);
// static void method(ir *c, class *class);
// static void method_body(ir *c, ir_t type, _key ar, class
// **class);

static void func_declaration(ir *c);
static void func_body(ir *c, _key ar);
static void func_var(ir *c);

static void var_dec(ir *c);
static void synchronize(parser *parser);

static void statement(ir *c);
static void print_statement(ir *c);

static void begin_scope(ir *c);
static void end_scope(ir *c);

static void parse_block(ir *c);
static void block(ir *c);

// static void comment(ir *c);

static void emit_loop(ir *c, int byte);
static int  emit_jump(ir *c, int byte);
static void patch_jump(ir *c, int byte);

static void for_statement(ir *c);
static void while_statement(ir *c);

static void rm_statement(ir *c);

static void consume_if(ir *c);
static void consume_elif(ir *c);
static void consume_switch(ir *c);

static void switch_statement(ir *c);
static void case_statement(ir *c);

static void if_statement(ir *c);
static void elif_statement(ir *c);
static void ternary_statement(ir *c);
static void null_coalescing_statement(ir *c);

static void return_statement(ir *c);

static void default_expression(ir *c);
static void expression(ir *c);

static bool match(token_t t, parser *parser);
static bool check(token_t t, parser *parser);
static bool is_comment(parser *parser);

static void   grouping(ir *c);
static PRule *get_rule(int t);

static void _and(ir *c);
static void _or(ir *c);

static void cast(ir *c);

static void binary(ir *c);
static void unary(ir *c);
static void infix_unary(ir *c);
static void compound_assign(ir *c);

static void current_err(const char *err, parser *parser);
static void prev_error(const char *err, parser *parser);
static void error_at(Token t, parser *parser, const char *err);

static void emit_byte(ir *c, uint8_t byte);
static void emit_bytes(ir *c, uint8_t b1, uint8_t b2);
static void emit_constant(ir *c, element ar);
static void emit_return(ir *c);

static void pi(ir *c);
static void euler(ir *c);
static void num(ir *c);
static void ch(ir *c);
static void boolean(ir *c);
static void str(ir *c);
static void fmt_str(ir *c);

static _key parse_string(ir *c);
static void stack_alloc(ir *c);

static void _table(ir *c);

static int resolve_local(ir *c, _key *name);
static int resolve_upvalue(ir *c, _key *name);
static int add_upvalue(ir *c, int upvalue, bool t);

static void parse_native_var_arg(ir *c);

static void dot(ir *c);
static void _this(ir *c);

static _key parse_id(ir *c);

static int  parse_var(ir *c, _key ar);
static void id(ir *c);

static bool idcmp(_key a, _key b);
static void declare_var(ir *c, _key ar);
static void add_local(ir *c, _key *ar);
*/
