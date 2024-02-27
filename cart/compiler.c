#include "compiler.h"
#include "compiler_util.h"
#ifdef DEBUG_TRACE_EXECUTION
#include "debug.h"
#endif
#include <stdio.h>
#include <stdlib.h>

static void init_compiler(Compiler *compiler)
{
    compiler->local_count = 0;
    compiler->scope_depth = 0;
    current = compiler;
}

static void consume(int t, const char *err)
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

    if (match(TOKEN_VAR))
        var_dec();
    else
        statement();

    if (parser.panic)
        synchronize();
}

static void var_dec()
{
    consume(TOKEN_ID, "Expect variable name.");

    arena ar = parse_id();
    int glob = parse_var(ar);

    uint8_t set = 0;
    if (glob == -1)
    {
        glob = resolve_local(current, &ar);
        set = OP_SET_LOCAL;
    }
    else
        set = OP_GLOBAL_DEF;

    if (match(TOKEN_OP_ASSIGN))
    {
        expression();
        emit_bytes(set, (uint8_t)glob);
    }
    else
        emit_byte(OP_NULL);
    consume(TOKEN_CH_SEMI,
            "Expect ';' after variable declaration.");
}

static void synchronize()
{
    parser.panic = false;

    while (parser.cur.type != TOKEN_EOF)
    {
        if (parser.pre.type == TOKEN_CH_SEMI)
            return;

        switch (parser.pre.type)
        {
        case TOKEN_FUNC:
        case TOKEN_CLASS:
        case TOKEN_FOR:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_PRINT:
        case TOKEN_RETURN:
            return;
        }
        advance_compiler();
    }
}

static void statement()
{

    if (match(TOKEN_PRINT))
        print_statement();
    else if (check(TOKEN_LINE_COMMENT) || check(TOKEN_NLINE_COMMENT))
    {
        advance_compiler();
        emit_byte(OP_NOOP);
    }
    else if (match(TOKEN_CH_LCURL))
    {
        begin_scope();
        block();
        end_scope();
    }
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

static void begin_scope()
{
    current->scope_depth++;
}
static void end_scope()
{
    current->scope_depth--;

    if (current->local_count > 0 && current->locals[current->local_count - 1].depth > current->scope_depth)
        emit_bytes(OP_POPN, add_constant(compile_chunk, Int(current->local_count)));

    while (current->local_count > 0 && current->locals[current->local_count - 1].depth > current->scope_depth)
        arena_free(&current->locals[--current->local_count].name);
}

static void block()
{
    while (!check(TOKEN_CH_RCURL) && !check(TOKEN_EOF))
        declaration();
    consume(TOKEN_CH_RCURL, "Expect `}` after block statement");
}

static bool match(int t)
{
    if (!check(t))
        return false;
    advance_compiler();
    return true;
}
static bool check(int t)
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
static PRule *get_rule(int t)
{
    return &rules[t];
}

static void parse_precedence(Precedence prec)
{
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
    int t = parser.pre.type;

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
    default:
        return;
    }
}
static void unary()
{
    int op = parser.pre.type;

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
    if (parser.panic)
        return;
    parser.panic = true;
    parser.err = true;

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
static void cstr()
{

    char *ch = (char *)++parser.pre.start;
    ch[parser.pre.size - 2] = '\0';

    emit_constant(String(ch));
}

static arena parse_id()
{
    char *ch = (char *)parser.pre.start;
    ch[parser.pre.size] = '\0';
    return Var(ch, machine.glob.capacity);
}
static void id()
{

    match(TOKEN_ID);
    arena ar = parse_id();
    uint8_t get, set;

    int arg = resolve_local(current, &ar);

    if (arg != -1)
    {
        get = OP_GET_LOCAL;
        set = OP_SET_LOCAL;
    }
    else
    {
        arg = add_constant(compile_chunk, ar);
        get = OP_GET_GLOBAL;
        set = OP_SET_GLOBAL;
    }

    if (match(TOKEN_OP_ASSIGN))
    {
        expression();
        emit_bytes(set, (uint8_t)arg);
    }
    else
        emit_bytes(get, (uint8_t)arg);
}
static int parse_var(arena ar)
{
    declare_var(ar);
    if (current->scope_depth > 0)
        return -1;
    return add_constant(compile_chunk, ar);
}

static bool idcmp(arena a, arena b)
{
    if (a.length != b.length)
        return false;

    return a.hash == b.hash;
}

static int resolve_local(Compiler *compiler, arena *name)
{
    for (int i = compiler->local_count - 1; i >= 0; i--)
        if (idcmp(*name, compiler->locals[i].name))
            return i;

    return -1;
}

static void declare_var(arena ar)
{
    if (current->scope_depth == 0)
        return;

    for (int i = current->local_count - 1; i >= 0; i--)
    {
        Local *local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scope_depth)
            break;

        if (idcmp(ar, local->name))
            error("Duplicate variable identifiers in scope");
    }

    add_local(&ar);
}

static void add_local(arena *ar)
{
    if (current->local_count == UINT8_COUNT)
    {
        error("Too many local variables in function.");
        return;
    }
    Local *local = &current->locals[current->local_count++];
    local->name = *ar;
    local->depth = current->scope_depth;
}
static void end_compile()
{
    emit_return();
#ifndef DEBUG_PRINT_CODE
    if (!parser.err)
        disassemble_chunk(compile_chunk, "code");
#endif
}
bool compile(const char *src, Chunk ch)
{
    Compiler compiler;

    init_scanner(src);
    init_compiler(&compiler);

    parser.panic = false;
    parser.err = false;
    compile_chunk = ch;

    advance_compiler();

    while (!match(TOKEN_EOF))
        declaration();
    consume(TOKEN_EOF, "Expect end of expression");

    end_compile();
    return !parser.err;
}