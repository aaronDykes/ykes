#include "compiler.h"
#include "compiler_util.h"
#include "arena_table.h"
#ifdef DEBUG_TRACE_EXECUTION
#include "debug.h"
#endif
#include <stdio.h>
#include <stdlib.h>

static void init_compiler(Compiler *a, Compiler *b, ObjType type, Arena name)
{

    Local *local = NULL;
    a->local_count = 0;
    a->scope_depth = 0;
    a->call_count = 0;

    if (b)
    {
        a->base = b->base;
        a->parser = b->parser;
        a->enclosing = b;
        a->scope_depth = b->scope_depth;
        local = &b->locals[b->local_count];
    }
    else
        local = &a->locals[a->local_count++];

    a->param_count = 0;
    a->upvalue_count = 0;
    a->func = function(name);
    a->type = type;

    local->depth = 0;
    local->name = Null();
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

    if (match(TOKEN_FUNC, &c->parser))
        func_declaration(c);
    else if (match(TOKEN_CLASS, &c->parser))
        class_declaration(c);
    else if (match(TOKEN_VAR, &c->parser))
        var_dec(c);
    else
        statement(c);

    if (c->parser.panic)
        synchronize(&c->parser);
}

static void class_declaration(Compiler *c)
{
    consume(TOKEN_CLASS, "Expect class keyword.", &c->parser);
    consume(TOKEN_ID, "Expect class name.", &c->parser);

    Arena ar = parse_id(c);
    int arg = parse_var(c, ar);

    emit_bytes(c, OP_CLASS, arg);
    consume(TOKEN_CH_LCURL, "Expect ze curl brace", &c->parser);
    // block(c);
}

static void call(Compiler *c)
{
    uint8_t argc = argument_list(c);
    emit_bytes(c, OP_CALL, argc);
}

static void call_expect_arity(Compiler *c, int arity)
{
    uint8_t argc = argument_list(c);
    if ((int)argc != arity)
        error("Incorrect number of args.", &c->parser);
    emit_bytes(c, OP_CALL, argc);
}

static int argument_list(Compiler *c)
{
    uint8_t argc = 0;

    if (match(TOKEN_CH_RPAREN, &c->parser))
        return 0;
    do
    {
        expression(c);
        if (argc == 255)
            current_err("Cannot pass more than 255 function parameters", &c->parser);
        argc++;

    } while (match(TOKEN_CH_COMMA, &c->parser));
    consume(TOKEN_CH_RPAREN, "Expect `)` after function args", &c->parser);
    return argc;
}

static void func_declaration(Compiler *c)
{
    consume(TOKEN_ID, "Expect function name.", &c->parser);
    Arena ar = parse_func_id(c);

    c->base->calls[c->base->call_count++] = ar;
    func_body(c, CLOSURE, ar);
}

static void func_body(Compiler *c, ObjType type, Arena ar)
{
    Compiler co;
    init_compiler(&co, c, type, ar);

    c = &co;
    consume(TOKEN_CH_LPAREN, "Expect `(` after function name.", &c->parser);
    if (!check(TOKEN_CH_RPAREN, &c->parser))
        do
        {
            c->func->arity++;
            if (c->func->arity > 255)
                current_err("Cannot declare more than 255 function parameters", &c->parser);
            func_var(c);

        } while (match(TOKEN_CH_COMMA, &c->parser));
    consume(TOKEN_CH_RPAREN, "Expect `)` after function parameters.", &c->parser);
    consume(TOKEN_CH_LCURL, "Expect `{` prior to function body.", &c->parser);

    c->func->params = GROW_TABLE(NULL, c->func->arity + 1);

    block(c);

    Compiler *tmp = c;
    Function *f = end_compile(c);

    Closure *clos = new_closure(f);

    c = c->enclosing;

    emit_bytes(
        c, OP_CLOSURE,
        add_constant(&c->func->ch, CLOSURE(clos)));

    for (int i = 0; i < tmp->upvalue_count; i++)
    {
        uint8_t local = tmp->upvalues[i].islocal ? 1 : 0;
        uint8_t index = (uint8_t)tmp->upvalues[i].index;

        emit_byte(c, local);
        emit_byte(c, index);
    }
}

static void func_var(Compiler *c)
{
    consume(TOKEN_ID, "Expect variable name.", &c->parser);

    Arena ar = parse_id(c);

    int glob = parse_var(c, ar);

    uint8_t set = 0;
    if (glob != -1)
        set = OP_FUNC_VAR_DEF;
    else
    {
        glob = resolve_local(c, &ar);
        set = OP_SET_LOCAL;
    }

    c->base->call_params[c->base->param_count++] = ar;
    if (match(TOKEN_OP_ASSIGN, &c->parser))
        expression(c);
    emit_bytes(c, set, (uint8_t)glob);
}

static void var_dec(Compiler *c)
{
    consume(TOKEN_ID, "Expect variable name.", &c->parser);

    Arena ar = parse_id(c);
    int glob = parse_var(c, ar);

    uint8_t set = 0;
    if (glob != -1)
        set = OP_GLOBAL_DEF;
    else
    {
        glob = resolve_local(c, &ar);
        set = OP_SET_LOCAL;
    }

    if (match(TOKEN_OP_ASSIGN, &c->parser))
    {
        expression(c);
        emit_bytes(c, set, (uint8_t)glob);
    }
    else
        emit_byte(c, OP_NULL);

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
    else if (match(TOKEN_OP_REM, &c->parser))
        rm_statement(c);
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
    else if (match(TOKEN_RETURN, &c->parser))
        return_statement(c);
    else if (is_comment(&c->parser))
        comment(c);
    else
        return default_expression(c);
}

static void rm_statement(Compiler *c)
{
    consume(TOKEN_CH_LPAREN, "Expect `(` prior to rm expression.", &c->parser);
    if (match(TOKEN_ID, &c->parser))
        ;

    Arena ar = parse_id(c);
    uint8_t get;
    int arg = resolve_local(c, &ar);

    if (arg != -1)
        get = OP_GET_LOCAL;
    else if ((arg = resolve_upvalue(c, &ar)) != -1)
        get = OP_GET_UPVALUE;
    else
    {
        arg = add_constant(&c->func->ch, OBJ(ar));
        get = OP_GET_GLOBAL;
    }
    emit_bytes(c, get, (uint8_t)arg);
    emit_byte(c, OP_RM);
    consume(TOKEN_CH_RPAREN, "Expect `)` after rm statement", &c->parser);
    consume(TOKEN_CH_SEMI, "Expect `;` at end of statement", &c->parser);
}

static inline bool is_comment(Parser *parser)
{
    return check(TOKEN_LINE_COMMENT, parser) || check(TOKEN_NLINE_COMMENT, parser);
}

static void comment(Compiler *c)
{

    advance_compiler(&c->parser);
    emit_byte(c, OP_NOOP);
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

    int start = c->func->ch.op_codes.count;
    int exit = -1;
    if (!match(TOKEN_CH_SEMI, &c->parser))
    {
        expression(c);
        consume(TOKEN_CH_SEMI, "Expect `;` after 'for' condition.", &c->parser);
        exit = emit_jump(c, OP_JMPF);
        emit_byte(c, OP_POP);
    }
    if (!match(TOKEN_CH_RPAREN, &c->parser))
    {
        int body_jump = emit_jump(c, OP_JMP);
        int inc_start = c->func->ch.op_codes.count;

        expression(c);
        emit_byte(c, OP_POP);
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
        emit_byte(c, OP_POP);
    }

    end_scope(c);
}

static void while_statement(Compiler *c)
{
    int start = c->func->ch.op_codes.count;

    consume(TOKEN_CH_LPAREN, "Expect `(` after 'while'.", &c->parser);
    expression(c);
    consume(TOKEN_CH_RPAREN, "Expect `)` after 'while' condition.", &c->parser);

    int exit_jmp = emit_jump(c, OP_JMPF);
    emit_byte(c, OP_POP);

    statement(c);

    // If falsey, loop
    emit_loop(c, start);

    // If true, exit
    patch_jump(c, exit_jmp);
    emit_byte(c, OP_POP);
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

static Arena consume_switch(Compiler *c)
{
    consume(TOKEN_CH_LPAREN, "Expect `(` after a 'switch'.", &c->parser);
    Arena args = get_id(c);
    consume(TOKEN_CH_RPAREN, "Expect `)` after a 'switch' condtion.", &c->parser);
    return args;
}

/**
 * TODO:
 *  Moving on... will revisit later
 */
static void switch_statement(Compiler *c)
{
    Arena args = consume_switch(c);
    bool expect = match(TOKEN_CH_LCURL, &c->parser);

    case_statement(c, args);

    if (match(TOKEN_DEFAULT, &c->parser))
    {
        consume(TOKEN_CH_COLON, "Expect `:` prior to case body.", &c->parser);
        statement(c);
    }
    if (expect)
        consume(TOKEN_CH_RCURL, "Expect `}` after switch body", &c->parser);

    c->func->ch.cases.listof.Ints[c->func->ch.cases.count++] = c->func->ch.op_codes.count;
}

static void case_statement(Compiler *c, Arena args)
{
    uint8_t get = args.listof.Ints[1];
    uint8_t arg = args.listof.Ints[0];

    while (match(TOKEN_CASE, &c->parser))
    {
        expression(c);
        consume(TOKEN_CH_COLON, "Expect `:` prior to case body.", &c->parser);
        emit_bytes(c, get, (uint8_t)arg);
        emit_byte(c, OP_SEQ);

        int tr = emit_jump_long(c, OP_JMPC);
        int begin = c->func->ch.op_codes.count;
        emit_byte(c, OP_POP);
        statement(c);
        emit_byte(c, OP_JMPL);
        emit_bytes(
            c,
            (c->func->ch.cases.count >> 8) & 0xFF,
            (c->func->ch.cases.count & 0xFF));
        patch_jump_long(c, begin, tr);
    }

    FREE_ARRAY(&args);
}

static void if_statement(Compiler *c)
{

    consume_if(c);

    int fi = emit_jump(c, OP_JMPF);
    // If truthy, follow through
    emit_byte(c, OP_POP);
    statement(c);

    int exit = emit_jump(c, OP_JMP);
    patch_jump(c, fi);
    emit_byte(c, OP_POP);

    elif_statement(c);

    if (match(TOKEN_ELSE, &c->parser))
        statement(c);

    c->func->ch.cases.listof.Ints[c->func->ch.cases.count++] = c->func->ch.op_codes.count;
    patch_jump(c, exit);
}

static void elif_statement(Compiler *c)
{
    while (match(TOKEN_ELIF, &c->parser))
    {
        consume_elif(c);
        int tr = emit_jump_long(c, OP_JMPC);
        int begin = c->func->ch.op_codes.count;
        emit_byte(c, OP_POP);
        statement(c);
        emit_byte(c, OP_JMPL);
        emit_bytes(
            c,
            (c->func->ch.cases.count >> 8) & 0xFF,
            (c->func->ch.cases.count & 0xFF));
        patch_jump_long(c, begin, tr);
    }
}

static void return_statement(Compiler *c)
{
    if (c->type == SCRIPT)
        error("Unable to return from top of script.", &c->parser);

    else if (match(TOKEN_CH_SEMI, &c->parser))
        emit_return(c);
    else
    {
        expression(c);
        consume(TOKEN_CH_SEMI, "Expect semi colon after return statement.", &c->parser);
        emit_byte(c, OP_RETURN);
    }
}

static void patch_jump_long(Compiler *c, int count, int offset)
{

    int j1 = count - offset - 4;
    int j2 = (c->func->ch.op_codes.count) - offset - 4;

    if (j1 >= INT16_MAX)
        error("To great a distance ", &c->parser);

    c->func->ch.op_codes.listof.Bytes[offset] = (uint8_t)((j2 >> 8) & 0xFF);
    c->func->ch.op_codes.listof.Bytes[offset + 1] = (uint8_t)(j2 & 0xFF);

    c->func->ch.op_codes.listof.Bytes[offset + 2] = (uint8_t)((j1 >> 8) & 0xFF);
    c->func->ch.op_codes.listof.Bytes[offset + 3] = (uint8_t)(j1 & 0xFF);
}

static void patch_jump(Compiler *c, int offset)
{

    int jump = c->func->ch.op_codes.count - offset - 2;

    if (jump >= INT16_MAX)
        error("To great a distance ", &c->parser);

    c->func->ch.op_codes.listof.Bytes[offset] = (uint8_t)((jump >> 8) & 0xFF);
    c->func->ch.op_codes.listof.Bytes[offset + 1] = (uint8_t)(jump & 0xFF);
}

static void emit_loop(Compiler *c, int byte)
{
    emit_byte(c, OP_LOOP);

    int offset = c->func->ch.op_codes.count - byte + 2;

    if (offset > UINT16_MAX)
        error("ERROR: big boi loop", &c->parser);

    emit_bytes(c, (offset >> 8) & 0xFF, offset & 0xFF);
}
static int emit_jump_long(Compiler *c, int byte)
{
    emit_byte(c, byte);
    emit_bytes(c, 0xFF, 0xFF);
    emit_bytes(c, 0xFF, 0xFF);

    return c->func->ch.op_codes.count - 4;
}
static int emit_jump(Compiler *c, int byte)
{
    emit_byte(c, byte);
    emit_bytes(c, 0xFF, 0xFF);

    return c->func->ch.op_codes.count - 2;
}

static void default_expression(Compiler *c)
{
    expression(c);
    consume(TOKEN_CH_SEMI, "Expect `;` after expression.", &c->parser);
    emit_byte(c, OP_POP);
}

static void print_statement(Compiler *c)
{
    expression(c);
    consume(TOKEN_CH_SEMI, "Expect ';' after value.", &c->parser);
    emit_byte(c, OP_PRINT);
}

static void begin_scope(Compiler *c)
{
    c->scope_depth++;
}
static void end_scope(Compiler *c)
{
    c->scope_depth--;

    if (c->local_count > 0 && (c->locals[c->local_count - 1].depth > c->scope_depth))
        emit_bytes(c, OP_POPN, add_constant(&c->func->ch, OBJ(Int(c->local_count))));

    while (c->local_count > 0 && (c->locals[c->local_count - 1].depth > c->scope_depth))
    {
        if (c->locals[c->local_count - 1].captured)
            emit_byte(c, OP_CLOSE_UPVAL);
        else
            arena_free(&c->locals[c->local_count - 1].name);
        --c->local_count;
    }
}

static void parse_block(Compiler *c)
{
    while (!check(TOKEN_CH_RCURL, &c->parser) && !check(TOKEN_EOF, &c->parser))
        declaration(c);
    consume(TOKEN_CH_RCURL, "Expect `}` after block statement", &c->parser);
}
static void block(Compiler *c)
{
    begin_scope(c);
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
    int end = emit_jump(c, OP_JMPF);

    emit_byte(c, OP_POP);
    parse_precedence(PREC_AND, c);

    patch_jump(c, end);
}
static void _or(Compiler *c)
{
    int else_jmp = emit_jump(c, OP_JMPT);

    emit_byte(c, OP_POP);
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
        emit_byte(c, OP_ADD);
        break;
    case TOKEN_OP_SUB:
        emit_byte(c, OP_SUB);
        break;
    case TOKEN_OP_MUL:
        emit_byte(c, OP_MUL);
        break;
    case TOKEN_OP_DIV:
        emit_byte(c, OP_DIV);
        break;
    case TOKEN_OP_MOD:
        emit_byte(c, OP_MOD);
        break;
    case TOKEN_OP_NE:
        emit_byte(c, OP_NE);
        break;
    case TOKEN_OP_EQ:
        emit_byte(c, OP_EQ);
        break;
    case TOKEN_OP_SNE:
        emit_byte(c, OP_SNE);
        break;
    case TOKEN_OP_SEQ:
        emit_byte(c, OP_SEQ);
        break;
    case TOKEN_OP_GT:
        emit_byte(c, OP_GT);
        break;
    case TOKEN_OP_GE:
        emit_byte(c, OP_GE);
        break;
    case TOKEN_OP_LT:
        emit_byte(c, OP_LT);
        break;
    case TOKEN_OP_LE:
        emit_byte(c, OP_LE);
        break;
    case TOKEN_OP_OR:
        emit_byte(c, OP_OR);
        break;
    case TOKEN_OP_AND:
        emit_byte(c, OP_AND);
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
        emit_byte(c, OP_NEG);
        break;
    case TOKEN_LINE_COMMENT:
    case TOKEN_NLINE_COMMENT:
        emit_byte(c, OP_NOOP);
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

static void emit_return(Compiler *c)
{
    emit_byte(c, OP_NULL);
    emit_byte(c, OP_RETURN);
}
static void emit_byte(Compiler *c, uint8_t byte)
{
    write_chunk(&c->func->ch, byte, c->parser.pre.line);
}
static void emit_bytes(Compiler *c, uint8_t b1, uint8_t b2)
{
    write_chunk(&c->func->ch, b1, c->parser.pre.line);
    write_chunk(&c->func->ch, b2, c->parser.pre.line);
}
static void emit_constant(Compiler *c, Arena ar)
{
    emit_bytes(
        c, OP_CONSTANT,
        add_constant(&c->func->ch, OBJ(ar)));
}

static void dval(Compiler *c)
{
    double val = strtod(c->parser.pre.start, NULL);
    emit_constant(c, Double(val));
}
static void ival(Compiler *c)
{
    emit_constant(c, Int(atoi(c->parser.pre.start)));
}
static void llint(Compiler *c)
{
    emit_constant(c, Long(atoll(c->parser.pre.start)));
}
static void ch(Compiler *c)
{
    emit_constant(c, Char(*++c->parser.pre.start));
}

static void boolean(Compiler *c)
{
    if (*c->parser.pre.start == 'n')
        emit_byte(c, OP_NULL);
    else
        emit_constant(c, Bool(*c->parser.pre.start == 't' ? true : false));
}
static void cstr(Compiler *c)
{

    char *ch = (char *)++c->parser.pre.start;
    ch[c->parser.pre.size - 2] = '\0';

    emit_constant(c, CString(ch));
}
static void string(Compiler *c)
{
    consume(TOKEN_CH_LPAREN, "Expect `(` prior to string declaration.", &c->parser);
    consume(TOKEN_STR, "Expect string declaration", &c->parser);
    char *ch = (char *)++c->parser.pre.start;
    ch[c->parser.pre.size - 2] = '\0';

    emit_constant(c, String(ch));
    consume(TOKEN_CH_RPAREN, "Expect `)` after string declaration.", &c->parser);
}

static void parse_native_argc0(Compiler *c)
{
    char *ch = (char *)c->parser.pre.start;
    ch[c->parser.pre.size] = '\0';
    int arg = add_constant(&c->func->ch, OBJ(native_name(ch)));
    emit_bytes(c, OP_GET_NATIVE, (uint8_t)arg);
    consume(TOKEN_CH_LPAREN, "Expect `(` prior to function call", &c->parser);
    call_expect_arity(c, 0);
}

static void parse_native_argc1(Compiler *c)
{

    char *ch = (char *)c->parser.pre.start;
    ch[c->parser.pre.size] = '\0';
    int arg = add_constant(&c->func->ch, OBJ(native_name(ch)));
    emit_bytes(c, OP_GET_NATIVE, (uint8_t)arg);
    consume(TOKEN_CH_LPAREN, "Expect `(` prior to function call", &c->parser);
    call_expect_arity(c, 1);
}

static Arena parse_func_id(Compiler *c)
{
    char *ch = (char *)c->parser.pre.start;
    ch[c->parser.pre.size] = '\0';
    return func_name(ch);
}

static Arena parse_id(Compiler *c)
{
    char *ch = (char *)c->parser.pre.start;
    ch[c->parser.pre.size] = '\0';
    return Var(ch);
}

static Arena get_id(Compiler *c)
{
    bool pre_inc = (c->parser.pre.type == TOKEN_OP_INC);
    bool pre_dec = (c->parser.pre.type == TOKEN_OP_DEC);

    if (match(TOKEN_ID, &c->parser))
        ;

    Arena ar = parse_id(c);

    uint8_t get, set;

    Arena args = GROW_ARRAY(NULL, 3 * sizeof(int), ARENA_INTS);
    int arg = resolve_local(c, &ar);

    if (arg != -1)
    {
        get = OP_GET_LOCAL;
        set = OP_SET_LOCAL;
    }
    else if ((arg = resolve_upvalue(c, &ar)) != -1)
    {
        get = OP_GET_UPVALUE;
        set = OP_SET_UPVALUE;
    }
    else
    {
        arg = add_constant(&c->func->ch, OBJ(ar));
        get = OP_GET_GLOBAL;
        set = OP_SET_GLOBAL;
    }
    args.listof.Ints[0] = arg;
    args.listof.Ints[1] = get;

    if (pre_inc)
        emit_bytes(c, get == OP_GET_LOCAL ? OP_INC_LOC : OP_INC_GLO, (uint8_t)arg);
    else if (pre_dec)
        emit_bytes(c, get == OP_GET_LOCAL ? OP_DEC_LOC : OP_DEC_GLO, (uint8_t)arg);
    else if (match(TOKEN_OP_DEC, &c->parser))
        emit_bytes(c, get == OP_GET_LOCAL ? OP_DEC_LOC : OP_DEC_GLO, (uint8_t)arg);
    else if (match(TOKEN_OP_INC, &c->parser))
        emit_bytes(c, get == OP_GET_LOCAL ? OP_INC_LOC : OP_INC_GLO, (uint8_t)arg);
    else if (match(TOKEN_OP_ASSIGN, &c->parser))
    {
        expression(c);
        emit_bytes(c, set, (uint8_t)arg);
    }
    else
        emit_bytes(c, get, (uint8_t)arg);
    return args;
}

static int resolve_call(Compiler *c, Arena *ar)
{

    for (int i = 0; i < c->base->call_count; i++)
        if (idcmp(*ar, c->base->calls[i]))
            return i;

    return -1;
}
static int resolve_call_param(Compiler *c, Arena *ar)
{

    for (int i = 0; i < c->base->param_count; i++)
        if (idcmp(*ar, c->base->call_params[i]))
            return i;

    return -1;
}

static void id(Compiler *c)
{
    bool pre_inc = (c->parser.pre.type == TOKEN_OP_INC);
    bool pre_dec = (c->parser.pre.type == TOKEN_OP_DEC);

    if (match(TOKEN_ID, &c->parser))
        ;

    Arena ar = parse_func_id(c);
    uint8_t get, set;
    int arg = 0;
    arg = resolve_call(c, &ar);

    if (arg != -1)
    {

        emit_bytes(c, OP_GET_CLOSURE, (uint8_t)arg);
        return;
    }

    ar.type = ARENA_VAR;
    arg = resolve_local(c, &ar);

    if (arg != -1)
    {
        get = OP_GET_LOCAL;
        set = OP_SET_LOCAL;
    }
    else if ((arg = resolve_upvalue(c, &ar)) != -1)
    {
        get = OP_GET_UPVALUE;
        set = OP_SET_UPVALUE;
    }
    else if ((arg = resolve_call_param(c, &ar)) != -1)
    {
        arg = add_constant(&c->func->ch, OBJ(ar));
        set = OP_SET_FUNC_VAR;
        get = OP_GET_FUNC_VAR;
    }
    else
    {
        arg = add_constant(&c->func->ch, OBJ(ar));
        get = OP_GET_GLOBAL;
        set = OP_SET_GLOBAL;
    }

    if (pre_inc)
        emit_bytes(c, get == OP_GET_LOCAL ? OP_INC_LOC : OP_INC_GLO, (uint8_t)arg);
    else if (pre_dec)
        emit_bytes(c, get == OP_GET_LOCAL ? OP_DEC_LOC : OP_DEC_GLO, (uint8_t)arg);
    else if (match(TOKEN_OP_DEC, &c->parser))
        emit_bytes(c, get == OP_GET_LOCAL ? OP_DEC_LOC : OP_DEC_GLO, (uint8_t)arg);
    else if (match(TOKEN_OP_INC, &c->parser))
        emit_bytes(c, get == OP_GET_LOCAL ? OP_INC_LOC : OP_INC_GLO, (uint8_t)arg);
    else if (match(TOKEN_OP_ASSIGN, &c->parser))
    {
        expression(c);
        emit_bytes(c, set, (uint8_t)arg);
    }
    else if (match(TOKEN_ADD_ASSIGN, &c->parser))
    {
        expression(c);
        emit_bytes(c, get, (uint8_t)arg);
        emit_byte(c, OP_ADD);
        emit_bytes(c, set, (uint8_t)arg);
    }
    else if (match(TOKEN_SUB_ASSIGN, &c->parser))
    {
        expression(c);
        emit_bytes(c, get, (uint8_t)arg);
        emit_byte(c, OP_SUB);
        emit_bytes(c, set, (uint8_t)arg);
    }
    else if (match(TOKEN_MUL_ASSIGN, &c->parser))
    {

        expression(c);
        emit_bytes(c, get, (uint8_t)arg);
        emit_byte(c, OP_MUL);
        emit_bytes(c, set, (uint8_t)arg);
    }
    else if (match(TOKEN_DIV_ASSIGN, &c->parser))
    {
        expression(c);
        emit_bytes(c, get, (uint8_t)arg);
        emit_byte(c, OP_DIV);
        emit_bytes(c, set, (uint8_t)arg);
    }
    else if (match(TOKEN_MOD_ASSIGN, &c->parser))
    {
        expression(c);
        emit_bytes(c, get, (uint8_t)arg);
        emit_byte(c, OP_MOD);
        emit_bytes(c, set, (uint8_t)arg);
    }
    else if (match(TOKEN_AND_ASSIGN, &c->parser))
    {
        expression(c);
        emit_bytes(c, get, (uint8_t)arg);
        emit_byte(c, OP_AND);
        emit_bytes(c, set, (uint8_t)arg);
    }
    else if (match(TOKEN_OR__ASSIGN, &c->parser))
    {
        expression(c);
        emit_bytes(c, get, (uint8_t)arg);
        emit_byte(c, OP_OR);
        emit_bytes(c, set, (uint8_t)arg);
    }
    else
        emit_bytes(c, get, (uint8_t)arg);
}

static int parse_var(Compiler *c, Arena ar)
{
    declare_var(c, ar);
    if (c->scope_depth > 0)
        return -1;
    return add_constant(&c->func->ch, OBJ(ar));
}

static bool idcmp(Arena a, Arena b)
{
    if (a.as.len != b.as.len)
        return false;

    return a.as.hash == b.as.hash;
}

static int resolve_local(Compiler *c, Arena *name)
{
    for (int i = c->local_count - 1; i >= 0; i--)
        if (idcmp(*name, c->locals[i].name))
            return i;
    return -1;
}
static int resolve_upvalue(Compiler *c, Arena *name)
{

    if (!c->enclosing)
        return -1;

    int local = resolve_local(c->enclosing, name);

    if (local != -1)
    {
        c->enclosing->locals[local].captured = true;
        return add_upvalue(c, (uint8_t)local, true);
    }

    int upvalue = resolve_upvalue(c->enclosing, name);
    if (upvalue != -1)
    {
        return add_upvalue(c, (uint8_t)upvalue, false);
    }

    return -1;
}

static int add_upvalue(Compiler *c, int index, bool islocal)
{
    int count = c->func->upvalue_count;

    for (int i = 0; i < count; i++)
    {
        Upvalue *upval = &c->upvalues[i];

        if (upval->index == index && upval->islocal == islocal)
            return i;
    }

    if (count > LOCAL_COUNT)
    {
        error("To many closure variables in function.", &c->parser);
        return 0;
    }

    c->upvalues[c->func->upvalue_count].islocal = islocal;
    c->upvalues[c->func->upvalue_count].index = (uint8_t)index;
    c->upvalue_count++;
    return c->func->upvalue_count++;
}

static void declare_var(Compiler *c, Arena ar)
{
    if (c->scope_depth == 0)
        return;

    for (int i = c->local_count - 1; i >= 0; i--)
    {
        Local *local = &c->locals[i];

        if (local->depth != 0 && local->depth < c->scope_depth)
            break;

        else if (idcmp(ar, local->name))
            error("Duplicate variable identifiers in scope", &c->parser);
    }

    add_local(c, &ar);
}

static void add_local(Compiler *c, Arena *ar)
{
    if (c->local_count == LOCAL_COUNT)
    {
        error("Too many local variables in function.", &c->parser);
        return;
    }
    c->locals[c->local_count].name = *ar;
    c->locals[c->local_count].depth = c->scope_depth;
    c->locals[c->local_count].captured = false;
    c->local_count++;
}
static Function *end_compile(Compiler *a)
{
    Function *f = a->func;

    // mark_compiler_roots(a);
    emit_return(a);
#ifdef DEBUG_PRINT_CODE
    if (!a->parser.err)
        disassemble_chunk(
            &a->func->ch,
            a->func->name.as.String);
#endif
    if (a->enclosing)
    {

        Parser tmp = a->parser;
        a = a->enclosing;
        a->parser = tmp;
    }

    return f;
}

Function *compile(const char *src)
{
    Compiler c;

    init_scanner(src);
    init_compiler(&c, NULL, SCRIPT, func_name("SCRIPT"));
    c.base = &c;

    c.parser.panic = false;
    c.parser.err = false;

    advance_compiler(&c.parser);

    while (!match(TOKEN_EOF, &c.parser))
        declaration(&c);
    consume(TOKEN_EOF, "Expect end of expression", &c.parser);

    Function *f = end_compile(&c);
    return c.parser.err ? NULL : f;
}