#include "compiler.h"
#include "compiler_util.h"
#ifdef DEBUG_TRACE_EXECUTION
#include "debug.h"
#endif
#include <stdio.h>
#include <stdlib.h>

static void init_compiler(comp *compiler)
{
    compiler->local_count = 0;
    compiler->scope_depth = 0;
}

static void consume(int t, const char *err, Parser *parser)
{
    if (parser->cur.type == t)
    {
        advance_compiler(parser);
        return;
    }
    current_err(err, parser);
}
static void advance_compiler(Parser *parser)
{
    parser->pre = parser->cur;

    for (;;)
    {
        parser->cur = scan_token();

        if (parser->cur.type != TOKEN_ERR)
            break;
        current_err(parser->cur.start, parser);
    }
}

static void declaration(Compiler *c)
{

    if (match(TOKEN_VAR, &c->parser))
        var_dec(c);
    else
        statement(c);

    if (c->parser.panic)
        synchronize(&c->parser);
}

static void var_dec(Compiler *c)
{
    consume(TOKEN_ID, "Expect variable name.", &c->parser);

    arena ar = parse_id(c);
    int glob = parse_var(c, ar);

    uint8_t set = 0;
    if (glob == -1)
    {
        glob = resolve_local(&c->compiler, &ar);
        set = OP_SET_LOCAL;
    }
    else
        set = OP_GLOBAL_DEF;

    if (match(TOKEN_OP_ASSIGN, &c->parser))
    {
        expression(c);
        emit_bytes(c->ch, set, (uint8_t)glob);
    }
    else
        emit_byte(c->ch, OP_NULL);

    consume(TOKEN_CH_SEMI, "Expect ';' after variable declaration.", &c->parser);
}

static void synchronize(Parser *parser)
{
    parser->panic = false;

    while (parser->cur.type != TOKEN_EOF)
    {
        if (parser->pre.type == TOKEN_CH_SEMI)
            return;

        switch (parser->pre.type)
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
        advance_compiler(parser);
    }
}

static void statement(Compiler *c)
{

    if (match(TOKEN_PRINT, &c->parser))
        print_statement(c);
    else if (match(TOKEN_IF, &c->parser))
        if_statement(c);
    else if (match(TOKEN_WHILE, &c->parser))
        while_statement(c);
    else if (match(TOKEN_FOR, &c->parser))
        for_statement(c);
    else if (match(TOKEN_SWITCH, &c->parser))
        switch_statement(c);
    else if (match(TOKEN_CH_LCURL, &c->parser))
        block(c);
    else if (is_comment(&c->parser))
        comment(c);
    else
        default_expression(c);
}

static inline bool is_comment(Parser *parser)
{
    return check(TOKEN_LINE_COMMENT, parser) || check(TOKEN_NLINE_COMMENT, parser);
}

static void comment(Compiler *c)
{

    advance_compiler(&c->parser);
    emit_byte(c->ch, OP_NOOP);
}

static void for_statement(Compiler *c)
{
    begin_scope(c);
    consume(TOKEN_CH_LPAREN, "Expect `(` after to 'for'.", &c->parser);
    if (!match(TOKEN_CH_SEMI, &c->parser))
    {
        if (match(TOKEN_VAR, &c->parser))
            var_dec(c);
        else
            id(c);
    }

    int start = c->ch->count;
    int exit = -1;
    if (!match(TOKEN_CH_SEMI, &c->parser))
    {
        expression(c);
        consume(TOKEN_CH_SEMI, "Expect `;` after 'for' condition.", &c->parser);
        exit = emit_jump(c->ch, OP_JMPF);
        emit_byte(c->ch, OP_POP);
    }
    if (!match(TOKEN_CH_RPAREN, &c->parser))
    {
        int body_jump = emit_jump(c->ch, OP_JMP);
        int inc_start = c->ch->count;

        expression(c);
        emit_byte(c->ch, OP_POP);
        consume(TOKEN_CH_RPAREN, "Expect `)` after 'for' statement.", &c->parser);

        emit_loop(c, start);
        start = inc_start;
        patch_jump(c, body_jump);
    }

    statement(c);
    emit_loop(c, start);

    if (exit != -1)
    {
        patch_jump(c, exit);
        emit_byte(c->ch, OP_POP);
    }

    end_scope(c);
}

static void while_statement(Compiler *c)
{
    int start = c->ch->count;

    consume(TOKEN_CH_LPAREN, "Expect `(` after 'while'.", &c->parser);
    expression(c);
    consume(TOKEN_CH_RPAREN, "Expect `)` after 'while' condition.", &c->parser);

    int exit_jmp = emit_jump(c->ch, OP_JMPF);
    emit_byte(c->ch, OP_POP);

    statement(c);

    // If falsey, loop
    emit_loop(c, start);

    // If true, exit
    patch_jump(c, exit_jmp);
    emit_byte(c->ch, OP_POP);
}

static void consume_if(Compiler *c)
{
    consume(TOKEN_CH_LPAREN, "Expect `(` after an 'if'.", &c->parser);
    expression(c);
    consume(TOKEN_CH_RPAREN, "Expect `)` after an 'if' condtion.", &c->parser);
}
static void consume_elif(Compiler *c)
{
    consume(TOKEN_CH_LPAREN, "Expect `(` after an 'elif'.", &c->parser);
    expression(c);
    consume(TOKEN_CH_RPAREN, "Expect `)` after an 'elif' condtion.", &c->parser);
}

/**
 * TODO:
 *  Moving on... will revisit later
 */
static void switch_statement(Compiler *c)
{
}

static void if_statement(Compiler *c)
{

    consume_if(c);

    int fi = emit_jump(c->ch, OP_JMPF);
    // If truthy, follow through
    emit_byte(c->ch, OP_POP);
    statement(c);

    int exit = emit_jump(c->ch, OP_JMP);
    patch_jump(c, fi);
    emit_byte(c->ch, OP_POP);

    if (check(TOKEN_ELIF, &c->parser))
        while (match(TOKEN_ELIF, &c->parser))
            elif_statement(c);
    else if (match(TOKEN_ELSE, &c->parser))
        statement(c);

    patch_jump(c, exit);
}

static void elif_statement(Compiler *c)
{
    consume_elif(c);
    int tr = emit_jump(c->ch, OP_ELIF);
    int elif = emit_jump(c->ch, OP_JMP);

    int begin = c->ch->count;
    emit_byte(c->ch, OP_POP);
    statement(c);
    patch_jump_end(c, begin, tr);

    patch_jump(c, elif);
    emit_byte(c->ch, OP_POP);
}

static void patch_jump_end(Compiler *c, int current, int begin)
{

    int j1 = current - begin - 2;

    if (j1 >= INT16_MAX)
        error("To great a distance ", &c->parser);

    c->ch->op_codes.as.Bytes[begin] = (uint8_t)((j1 >> 8) & 0xFF);
    c->ch->op_codes.as.Bytes[begin + 1] = (uint8_t)(j1 & 0xFF);
}

static void patch_jump(Compiler *c, int offset)
{

    int jump = c->ch->count - offset - 2;

    if (jump >= INT16_MAX)
        error("To great a distance ", &c->parser);

    c->ch->op_codes.as.Bytes[offset] = (uint8_t)((jump >> 8) & 0xFF);
    c->ch->op_codes.as.Bytes[offset + 1] = (uint8_t)(jump & 0xFF);
}

static void emit_loop(Compiler *c, int byte)
{
    emit_byte(c->ch, OP_LOOP);

    int offset = c->ch->count - byte + 2;

    if (offset > UINT16_MAX)
        error("ERROR: big boi loop", &c->parser);

    emit_bytes(c->ch, (offset >> 8) & 0xFF, offset & 0xFF);
}
static int emit_jumps(Chunk ch, int byte)
{
    emit_byte(ch, byte);
    emit_bytes(ch, 0xFF, 0xFF);
    emit_bytes(ch, 0xFF, 0xFF);

    return ch->count - 4;
}
static int emit_jump(Chunk ch, int byte)
{
    emit_byte(ch, byte);
    emit_bytes(ch, 0xFF, 0xFF);

    return ch->count - 2;
}

static void default_expression(Compiler *c)
{
    expression(c);
    consume(TOKEN_CH_SEMI, "Expect `;` after expression.", &c->parser);
    emit_byte(c->ch, OP_POP);
}

static void print_statement(Compiler *c)
{
    expression(c);
    consume(TOKEN_CH_SEMI, "Expect ';' after value.", &c->parser);
    emit_byte(c->ch, OP_PRINT);
}

static void begin_scope(comp *compiler)
{
    compiler->scope_depth++;
}
static void end_scope(Compiler *c)
{
    c->compiler.scope_depth--;

    if (c->compiler.local_count > 0 && c->compiler.locals[c->compiler.local_count - 1].depth > c->compiler.scope_depth)
        emit_bytes(c->ch, OP_POPN, add_constant(c->ch, Int(c->compiler.local_count)));

    while (c->compiler.local_count > 0 && c->compiler.locals[c->compiler.local_count - 1].depth > c->compiler.scope_depth)
        arena_free(&c->compiler.locals[--c->compiler.local_count].name);
}

static void parse_block(Compiler *c)
{
    while (!check(TOKEN_CH_RCURL, &c->parser) && !check(TOKEN_EOF, &c->parser))
        declaration(c);
    consume(TOKEN_CH_RCURL, "Expect `}` after block statement", &c->parser);
}
static void block(Compiler *c)
{
    begin_scope(&c->compiler);
    parse_block(c);
    end_scope(c);
}

static bool match(int t, Parser *parser)
{
    if (!check(t, parser))
        return false;
    advance_compiler(parser);
    return true;
}
static bool check(int t, Parser *parser)
{
    return parser->cur.type == t;
}

static void expression(Compiler *c)
{
    parse_precedence(PREC_ASSIGNMENT, c);
}
static void grouping(Compiler *c)
{
    expression(c);
    consume(TOKEN_CH_RPAREN, "Expect `)` after expression", &c->parser);
}
static PRule *get_rule(int t)
{
    return &rules[t];
}

static void parse_precedence(Precedence prec, Compiler *c)
{
    advance_compiler(&c->parser);
    parse_fn prefix_rule = get_rule(c->parser.pre.type)->prefix;

    if (!prefix_rule)
    {
        error("Expect expression.", &c->parser);
        return;
    }

    prefix_rule(c);

    while (prec <= get_rule(c->parser.cur.type)->prec)
    {
        advance_compiler(&c->parser);
        parse_fn infix = get_rule(c->parser.pre.type)->infix;
        infix(c);
    }
}

static void _and(Compiler *c)
{
    int end = emit_jump(c->ch, OP_JMPF);

    emit_byte(c->ch, OP_POP);
    parse_precedence(PREC_AND, c);

    patch_jump(c, end);
}
static void _or(Compiler *c)
{
    int else_jmp = emit_jump(c->ch, OP_JMPT);

    emit_byte(c->ch, OP_POP);
    parse_precedence(PREC_OR, c);

    patch_jump(c, else_jmp);
}

static void binary(Compiler *c)
{
    int t = c->parser.pre.type;

    PRule *rule = get_rule(t);
    parse_precedence((Precedence)rule->prec + 1, c);

    switch (t)
    {
    case TOKEN_OP_ADD:
        emit_byte(c->ch, OP_ADD);
        break;
    case TOKEN_OP_SUB:
        emit_byte(c->ch, OP_SUB);
        break;
    case TOKEN_OP_MUL:
        emit_byte(c->ch, OP_MUL);
        break;
    case TOKEN_OP_DIV:
        emit_byte(c->ch, OP_DIV);
        break;
    case TOKEN_OP_MOD:
        emit_byte(c->ch, OP_MOD);
        break;
    case TOKEN_OP_NE:
        emit_byte(c->ch, OP_NE);
        break;
    case TOKEN_OP_EQ:
        emit_byte(c->ch, OP_EQ);
        break;
    case TOKEN_OP_SNE:
        emit_byte(c->ch, OP_SNE);
        break;
    case TOKEN_OP_SEQ:
        emit_byte(c->ch, OP_SEQ);
        break;
    case TOKEN_OP_GT:
        emit_byte(c->ch, OP_GT);
        break;
    case TOKEN_OP_GE:
        emit_byte(c->ch, OP_GE);
        break;
    case TOKEN_OP_LT:
        emit_byte(c->ch, OP_LT);
        break;
    case TOKEN_OP_LE:
        emit_byte(c->ch, OP_LE);
        break;
    case TOKEN_OP_OR:
        emit_byte(c->ch, OP_OR);
        break;
    case TOKEN_OP_AND:
        emit_byte(c->ch, OP_AND);
        break;
    default:
        return;
    }
}
static void unary(Compiler *c)
{
    int op = c->parser.pre.type;

    parse_precedence(PREC_UNARY, c);

    switch (op)
    {
    case TOKEN_OP_SUB:
    case TOKEN_OP_BANG:
        emit_byte(c->ch, OP_NEG);
        break;
    case TOKEN_LINE_COMMENT:
    case TOKEN_NLINE_COMMENT:
        emit_byte(c->ch, OP_NOOP);
        break;
    default:
        return;
    }
}

static void current_err(const char *err, Parser *parser)
{
    error_at(&parser->cur, parser, err);
}
static void error(const char *err, Parser *parser)
{
    error_at(&parser->pre, parser, err);
}
static void error_at(Token toke, Parser *parser, const char *err)
{
    if (parser->panic)
        return;
    parser->panic = true;
    parser->err = true;

    fprintf(stderr, "[line %d] Error", toke->line);

    if (toke->type == TOKEN_EOF)
        fprintf(stderr, " at end");
    else if (toke->type != TOKEN_ERR)
        fprintf(stderr, " at '%.*s'", toke->size, toke->start);

    fprintf(stderr, ": %s\n", err);
}

static void emit_return(Chunk ch)
{
    emit_byte(ch, OP_RETURN);
}
static void emit_byte(Chunk ch, uint8_t byte)
{
    write_chunk(ch, byte);
}
static void emit_bytes(Chunk ch, uint8_t b1, uint8_t b2)
{
    write_chunk(ch, b1);
    write_chunk(ch, b2);
}
static void emit_constant(Chunk ch, arena ar)
{
    emit_bytes(ch, OP_CONSTANT, add_constant(ch, ar));
}

static void dval(Compiler *c)
{
    double val = strtod(c->parser.pre.start, NULL);
    emit_constant(c->ch, Double(val));
}
static void ival(Compiler *c)
{
    emit_constant(c->ch, Int(atoi(c->parser.pre.start)));
}
static void llint(Compiler *c)
{
    emit_constant(c->ch, Long(atoll(c->parser.pre.start)));
}
static void ch(Compiler *c)
{
    emit_constant(c->ch, Char(*++c->parser.pre.start));
}

static void boolean(Compiler *c)
{
    if (*c->parser.pre.start == 'n')
        emit_byte(c->ch, OP_NULL);
    else
        emit_constant(c->ch, Bool(*c->parser.pre.start == 't' ? true : false));
}
static void cstr(Compiler *c)
{

    char *ch = (char *)++c->parser.pre.start;
    ch[c->parser.pre.size - 2] = '\0';

    emit_constant(c->ch, String(ch));
}

static arena parse_id(Compiler *c)
{
    char *ch = (char *)c->parser.pre.start;
    ch[c->parser.pre.size] = '\0';
    return Var(ch, c->machine->glob.capacity);
}
static void id(Compiler *c)
{

    match(TOKEN_ID, &c->parser);
    arena ar = parse_id(c);
    uint8_t get, set;

    int arg = resolve_local(&c->compiler, &ar);

    if (arg != -1)
    {
        get = OP_GET_LOCAL;
        set = OP_SET_LOCAL;
    }
    else
    {
        arg = add_constant(c->ch, ar);
        get = OP_GET_GLOBAL;
        set = OP_SET_GLOBAL;
    }

    if (match(TOKEN_OP_ASSIGN, &c->parser))
    {
        expression(c);
        emit_bytes(c->ch, set, (uint8_t)arg);
    }
    else
        emit_bytes(c->ch, get, (uint8_t)arg);
}

static int parse_var(Compiler *c, arena ar)
{
    declare_var(c, ar);
    if (c->compiler.scope_depth > 0)
        return -1;
    return add_constant(c->ch, ar);
}

static bool idcmp(arena a, arena b)
{
    if (a.length != b.length)
        return false;

    return a.hash == b.hash;
}

static int resolve_local(comp *c, arena *name)
{
    for (int i = c->local_count - 1; i >= 0; i--)
        if (idcmp(*name, c->locals[i].name))
            return i;

    return -1;
}

static void declare_var(Compiler *c, arena ar)
{
    if (c->compiler.scope_depth == 0)
        return;

    for (int i = c->compiler.local_count - 1; i >= 0; i--)
    {
        Local *local = &c->compiler.locals[i];
        if (local->depth != -1 && local->depth < c->compiler.scope_depth)
            break;

        if (idcmp(ar, local->name))
            error("Duplicate variable identifiers in scope", &c->parser);
    }

    add_local(c, &ar);
}

static void add_local(Compiler *c, arena *ar)
{
    if (c->compiler.local_count == LOCAL_COUNT)
    {
        error("Too many local variables in function.", &c->parser);
        return;
    }
    Local *local = &c->compiler.locals[c->compiler.local_count++];
    local->name = *ar;
    local->depth = c->compiler.scope_depth;
}
static void end_compile(Chunk ch)
{
    emit_return(ch);
#ifndef DEBUG_PRINT_CODE
    if (!parser.err)
        disassemble_chunk(compile_chunk, "code");
#endif
}
bool compile(const char *src, Chunk ch, vm *machine)
{
    Compiler c;

    init_scanner(src);
    init_compiler(&c.compiler);

    c.ch = ch;
    c.parser.panic = false;
    c.parser.err = false;
    c.machine = machine;

    advance_compiler(&c.parser);

    while (!match(TOKEN_EOF, &c.parser))
        declaration(&c);
    consume(TOKEN_EOF, "Expect end of expression", &c.parser);

    end_compile(c.ch);
    return !c.parser.err;
}