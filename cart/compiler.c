#include "compiler.h"
#include "table.h"
#ifdef DEBUG_TRACE_EXECUTION
#include "debug.h"
#endif
#include <stdio.h>
#include <stdlib.h>

static Parser parser;
static Chunk compile_chunk;

static void consume(token_type t, const char *str);
static void advance_compiler();

static void declaration();
static void statement();
static void print_statement();

static bool match(token_type t);
static bool check(token_type t);
static void expression();
static void grouping();
static PRule *get_rule(token_type t);
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
static void id();
static void cstr();

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

    [TOKEN_OP_ASSIGN] = {NULL, binary, PREC_ASSIGNMENT},
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

static void consume(token_type t, const char *err)
{
    if (parser.cur.type == t)
    {
        advance_compiler();
        return;
    }
    current_err(err);
}
static void advance_compiler()
{
    parser.pre = parser.cur;

    for (;;)
    {
        parser.cur = scan_token();

        if (parser.cur.type != TOKEN_ERR)
            break;
        current_err(parser.cur.start);
    }
}

static void declaration()
{
    statement();
}
static void statement()
{
    if (match(TOKEN_PRINT))
        print_statement();
    else
    {
        expression();
        consume(TOKEN_CH_SEMI, "Expect `;` after expression.");
        emit_byte(OP_POP);
    }
}

static void print_statement()
{
    expression();
    consume(TOKEN_CH_SEMI, "Expect ';' after value.");
    emit_byte(OP_PRINT);
}

static bool match(token_type t)
{
    if (!check(t))
        return false;
    advance_compiler();
    return true;
}
static bool check(token_type t)
{
    return parser.cur.type == t;
}

static void expression()
{
    parse_precedence(PREC_ASSIGNMENT);
}
static void grouping()
{
    expression();
    consume(TOKEN_CH_RPAREN, "Expect `)` after expression");
}
static PRule *get_rule(token_type t)
{
    return &rules[t];
}

static void parse_precedence(Precedence prec)
{
    if ((parser.pre.type == TOKEN_NLINE_COMMENT || parser.pre.type == TOKEN_LINE_COMMENT) && (parser.cur.type == TOKEN_EOF))
        return;
    advance_compiler();

    parse_fn prefix_rule = get_rule(parser.pre.type)->prefix;

    if (!prefix_rule)
    {
        error("Expect expression.");
        return;
    }

    prefix_rule();

    while (prec <= get_rule(parser.cur.type)->prec)
    {
        advance_compiler();
        parse_fn infix = get_rule(parser.pre.type)->infix;
        infix();
    }
}

static void binary()
{
    token_type t = parser.pre.type;

    PRule *rule = get_rule(t);
    parse_precedence((Precedence)rule->prec + 1);

    switch (t)
    {
    case TOKEN_OP_ADD:
        emit_byte(OP_ADD);
        break;
    case TOKEN_OP_SUB:
        emit_byte(OP_SUB);
        break;
    case TOKEN_OP_MUL:
        emit_byte(OP_MUL);
        break;
    case TOKEN_OP_DIV:
        emit_byte(OP_DIV);
        break;
    case TOKEN_OP_MOD:
        emit_byte(OP_MOD);
        break;
    case TOKEN_OP_NE:
        emit_byte(OP_NE);
        break;
    case TOKEN_OP_EQ:
        emit_byte(OP_EQ);
        break;
    case TOKEN_OP_GT:
        emit_byte(OP_GT);
        break;
    case TOKEN_OP_GE:
        emit_byte(OP_GE);
        break;
    case TOKEN_OP_LT:
        emit_byte(OP_LT);
        break;
    case TOKEN_OP_LE:
        emit_byte(OP_LE);
        break;
    case TOKEN_OP_OR:
        emit_byte(OP_OR);
        break;
    case TOKEN_OP_AND:
        emit_byte(OP_AND);
        break;
    case TOKEN_OP_ASSIGN:
        emit_byte(OP_ASSIGN);
        break;
    default:
        return;
    }
}
static void unary()
{
    token_type op = parser.pre.type;

    parse_precedence(PREC_UNARY);

    switch (op)
    {
    case TOKEN_OP_SUB:
    case TOKEN_OP_BANG:
        emit_byte(OP_NEG);
        break;
    case TOKEN_LINE_COMMENT:
    case TOKEN_NLINE_COMMENT:
        emit_byte(OP_NOOP);
        break;
    default:
        return;
    }
}

static void current_err(const char *err)
{
    error_at(&parser.cur, err);
}
static void error(const char *err)
{
    error_at(&parser.pre, err);
}
static void error_at(Token toke, const char *err)
{
    if (parser.fuck)
        return;
    parser.fuck = true;
    parser.had_err = true;

    fprintf(stderr, "[line %d] Error", toke->line);

    if (toke->type == TOKEN_EOF)
        fprintf(stderr, " at end");
    else if (toke->type != TOKEN_ERR)
        fprintf(stderr, " at '%.*s'", toke->size, toke->start);

    fprintf(stderr, ": %s\n", err);
}

static void emit_return()
{
    emit_byte(OP_RETURN);
}
static void emit_byte(uint8_t byte)
{
    write_chunk(compile_chunk, byte);
}
static void emit_bytes(uint8_t b1, uint8_t b2)
{
    write_chunk(compile_chunk, b1);
    write_chunk(compile_chunk, b2);
}
static void emit_constant(arena ar)
{
    emit_bytes(OP_CONSTANT, add_constant(compile_chunk, ar));
}

static void dval()
{
    double val = strtod(parser.pre.start, NULL);
    emit_constant(Double(val));
}
static void ival()
{
    emit_constant(Int(atoi(parser.pre.start)));
}
static void llint()
{
    emit_constant(Long(atoll(parser.pre.start)));
}
static void ch()
{
    emit_constant(Char(*++parser.pre.start));
}

static void boolean()
{
    if (*parser.pre.start == 'n')
        emit_byte(OP_NULL);
    else
        emit_constant(Bool(*parser.pre.start == 't' ? true : false));
}
static void id()
{

    char *ch = parser.pre.start;
    ch[parser.pre.size] = '\0';
    arena tmp = Var(ch, machine.d.capacity);

    emit_constant(tmp);
}

static void cstr()
{

    char *ch = ++parser.pre.start;
    ch[parser.pre.size - 2] = '\0';

    emit_constant(String(ch));
}

static void end_compile()
{
    emit_return();
#ifndef DEBUG_PRINT_CODE
    if (!parser.had_err)
        disassemble_chunk(compile_chunk, "code");
#endif
}
bool compile(const char *src, Chunk ch)
{
    init_scanner(src);
    parser.fuck = false;
    parser.had_err = false;
    compile_chunk = ch;

    advance_compiler();

    while (!match(TOKEN_EOF))
        declaration();
    consume(TOKEN_EOF, "Expect end of expression");

    end_compile();
    return !parser.had_err;
}