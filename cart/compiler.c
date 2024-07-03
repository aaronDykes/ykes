#include "compiler.h"
#include "compiler_util.h"
#include "arena_table.h"
#ifdef DEBUG_TRACE_EXECUTION
#include "debug.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

static void init_compiler(compiler *a, compiler *b, ObjType type, arena name)
{

    local *_local = NULL;
    a->meta.cwd = NULL;
    a->meta.type = type;

    a->count.local = 0;
    a->count.scope = 0;
    a->count.call = 0;
    a->count.class = 0;

    a->array.index = 0;
    a->array.set = 0;
    a->array.get = 0;

    a->count.upvalue = 0;

    a->lookup.call = NULL;
    a->lookup.native = NULL;
    a->lookup.class = NULL;
    a->lookup.include = NULL;

    a->ykes.class_compiler = NULL;
    a->ykes.func = NULL;
    a->ykes.func = _function(name);

    if (b)
    {
        a->ykes.base = b->ykes.base;
        a->ykes.parser = b->ykes.parser;
        a->ykes.enclosing = b;
        a->count.scope = b->count.scope;
        a->ykes.class_compiler = b->ykes.class_compiler;

        _local = &b->stack.local[b->count.local++];
    }
    else
        _local = &a->stack.local[a->count.local++];

    _local->depth = 0;
    _local->captured = false;

    _local->name = (type == METHOD)
                       ? String("this")
                       : Null();
}

static void consume(int t, const char *err, parser *parser)
{
    if (parser->cur.type == t)
    {
        advance_compiler(parser);
        return;
    }
    current_err(err, parser);
}
static void advance_compiler(parser *parser)
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
static char *read_file(const char *path)
{

    FILE *file = fopen(path, "rb");

    if (!file)
    {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char *buffer = NULL;
    buffer = ALLOC(fileSize + 1);

    if (!buffer)
    {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

static bool resolve_include(compiler *c, arena ar)
{

    element el = find_entry(&c->ykes.base->lookup.include, &ar);

    if (el.type != NULL_OBJ)
        return true;

    return false;
}

static void str_cop(char *src, char *dst)
{
    char *tmp = src;

    while ((*tmp++ = *dst++))
        ;
}
static char *get_name(char *path)
{
    int len = strlen(path) - 1;
    char *tmp = path + len;

    int count;
    for (count = 0; tmp[-1] != '/'; --tmp, count++)
        ;

    char *file = NULL;
    file = ALLOC((count + 1) * sizeof(char));

    strcpy(file, tmp);

    return file;
}
static void include_file(compiler *c)
{

#define SIZE(x, y) \
    strlen(x) + strlen(y)

    if (c->meta.type != SCRIPT)
    {
        error("Can only include files at top level.", &c->ykes.parser);
        exit(1);
    }

    char *remaining = NULL;

    consume(TOKEN_STR, "Expect file path.", &c->ykes.parser);
    arena inc = CString(parse_string(c));

    if (resolve_include(c, inc))
    {
        error("Double include.", &c->ykes.parser);
        exit(1);
    }

    write_table(c->ykes.base->lookup.include, inc, OBJ(inc));

    consume(TOKEN_CH_SEMI, "Expect `;` at end of include statement.", &c->ykes.parser);
    remaining = (char *)c->ykes.parser.cur.start;

    char path[CWD_MAX] = {0};

    str_cop(path, (char *)c->ykes.base->meta.cwd);
    strcat(path, inc.as.String);

    char *file = read_file(path);

    arena result = GROW_ARENA(
        NULL,
        SIZE(file, remaining),
        ARENA_STR);

    str_cop(result.as.String, file);

    strcat(result.as.String, remaining);
    init_scanner(result.as.String);
    c->ykes.parser.cur = scan_token();

    c->ykes.parser.current_file = get_name(inc.as.String);

#undef SIZE
}

static void declaration(compiler *c)
{

    if (match(TOKEN_INCLUDE, &c->ykes.parser))
        include_file(c);
    else if (match(TOKEN_FUNC, &c->ykes.parser))
        func_declaration(c);
    else if (match(TOKEN_CLASS, &c->ykes.parser))
        class_declaration(c);
    else if (match(TOKEN_VAR, &c->ykes.parser))
        var_dec(c);
    else
        statement(c);

    if (c->ykes.parser.panic)
        synchronize(&c->ykes.parser);
}

static void class_declaration(compiler *c)
{
    consume(TOKEN_ID, "ERROR: Expect class name.", &c->ykes.parser);

    arena ar = parse_id(c);
    class *classc = _class(ar);
    classc->closures = GROW_TABLE(NULL, STACK_SIZE);
    class_compiler *cc = ALLOC(sizeof(class_compiler));

    write_table(c->ykes.base->lookup.class, classc->name, OBJ(Int(c->ykes.base->count.class ++)));
    c->ykes.base->stack.instance[c->ykes.base->count.class - 1] = classc;
    cc->instance_name = ar;

    cc->enclosing = c->ykes.class_compiler;
    c->ykes.class_compiler = cc;

    emit_bytes(c, OP_CLASS, add_constant(&c->ykes.func->ch, CLASS(classc)));

    consume(TOKEN_CH_LCURL, "ERROR: Expect ze `{` curl brace", &c->ykes.parser);

    while (!check(TOKEN_CH_RCURL, &c->ykes.parser) && !check(TOKEN_EOF, &c->ykes.parser))
        method(c, classc);

    consume(TOKEN_CH_RCURL, "ERROR: Expect ze `}` curl brace", &c->ykes.parser);
    c->ykes.class_compiler = c->ykes.class_compiler->enclosing;
}

static void method(compiler *c, class *class)
{
    consume(TOKEN_ID, "ERROR: Expect method identifier.", &c->ykes.parser);
    arena ar = parse_func_id(c);

    ObjType type = INIT;

    if (ar.as.hash != c->ykes.base->_hash_ref.init.as.hash)
        type = METHOD;

    method_body(c, type, ar, &class);
}

static void method_body(compiler *c, ObjType type, arena ar, class **class)
{
    compiler co;
    init_compiler(&co, c, type, ar);

    c = &co;
    begin_scope(c);
    consume(TOKEN_CH_LPAREN, "Expect `(` after function name.", &c->ykes.parser);
    if (!check(TOKEN_CH_RPAREN, &c->ykes.parser))
        do
        {
            c->ykes.func->arity++;
            if (c->ykes.func->arity > 255)
                current_err("Cannot declare more than 255 function parameters", &c->ykes.parser);
            func_var(c);

        } while (match(TOKEN_CH_COMMA, &c->ykes.parser));
    consume(TOKEN_CH_RPAREN, "Expect `)` after function parameters.", &c->ykes.parser);
    consume(TOKEN_CH_LCURL, "Expect `{` prior to function body.", &c->ykes.parser);

    parse_block(c);

    compiler *tmp = c;
    function *f = end_compile(c);

    closure *clos = _closure(f);

    write_table((*class)->closures, ar, CLOSURE(clos));

    end_scope(c);

    if (type == INIT)
        (*class)->init = clos;

    c = c->ykes.enclosing;

    emit_bytes(
        c, OP_METHOD,
        add_constant(&c->ykes.func->ch, CLOSURE(clos)));

    for (int i = 0; i < tmp->count.upvalue; i++)
    {
        emit_byte(c, tmp->stack.upvalue[i].islocal ? 1 : 0);
        emit_byte(c, (uint8_t)tmp->stack.upvalue[i].index);
    }
}

static void call(compiler *c)
{
    uint8_t argc = argument_list(c);
    emit_bytes(c, OP_CALL, argc);
}

static int argument_list(compiler *c)
{
    uint8_t argc = 0;

    if (match(TOKEN_CH_RPAREN, &c->ykes.parser))
        return 0;
    do
    {
        expression(c);
        if (argc == 255)
            current_err("Cannot pass more than 255 function parameters", &c->ykes.parser);
        argc++;

    } while (match(TOKEN_CH_COMMA, &c->ykes.parser));
    consume(TOKEN_CH_RPAREN, "Expect `)` after function args", &c->ykes.parser);
    return argc;
}

static void func_declaration(compiler *c)
{
    consume(TOKEN_ID, "Expect function name.", &c->ykes.parser);
    arena ar = parse_func_id(c);

    write_table(c->ykes.base->lookup.call, ar, OBJ(Int(c->ykes.base->count.call++)));
    func_body(c, CLOSURE, ar);
}

static void func_body(compiler *c, ObjType type, arena ar)
{
    compiler co;
    init_compiler(&co, c, type, ar);

    c = &co;
    begin_scope(c);
    consume(TOKEN_CH_LPAREN, "Expect `(` after function name.", &c->ykes.parser);
    if (!check(TOKEN_CH_RPAREN, &c->ykes.parser))
        do
        {
            c->ykes.func->arity++;
            if (c->ykes.func->arity > 255)
                current_err("Cannot declare more than 255 function parameters", &c->ykes.parser);
            func_var(c);

        } while (match(TOKEN_CH_COMMA, &c->ykes.parser));
    consume(TOKEN_CH_RPAREN, "Expect `)` after function parameters.", &c->ykes.parser);
    consume(TOKEN_CH_LCURL, "Expect `{` prior to function body.", &c->ykes.parser);

    parse_block(c);

    compiler *tmp = c;
    function *f = end_compile(c);

    closure *clos = _closure(f);
    end_scope(c);

    c = c->ykes.enclosing;

    emit_bytes(
        c, OP_CLOSURE,
        add_constant(&c->ykes.func->ch, CLOSURE(clos)));

    for (int i = 0; i < tmp->count.upvalue; i++)
    {
        emit_byte(c, tmp->stack.upvalue[i].islocal ? 1 : 0);
        emit_byte(c, (uint8_t)tmp->stack.upvalue[i].index);
    }
}

static void func_var(compiler *c)
{
    consume(TOKEN_ID, "Expect variable name.", &c->ykes.parser);

    arena ar = parse_id(c);

    int glob = parse_var(c, ar);

    uint8_t set = OP_SET_FUNC_VAR;

    if (glob == -1)
    {
        glob = resolve_local(c, &ar);
        set = OP_SET_LOCAL_PARAM;
    }
    emit_bytes(c, set, (uint8_t)glob);
}

static void var_dec(compiler *c)
{
    consume(TOKEN_ID, "Expect variable name.", &c->ykes.parser);

    arena ar = parse_id(c);
    int glob = parse_var(c, ar);

    uint8_t set = 0;
    if (glob != -1)
        set = OP_GLOBAL_DEF;
    else
    {
        glob = resolve_local(c, &ar);
        set = OP_SET_LOCAL;
    }

    if (match(TOKEN_OP_ASSIGN, &c->ykes.parser))
    {
        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);
        emit_bytes(c, set, (uint8_t)glob);
    }
    else
        emit_byte(c, OP_NULL);

    consume(TOKEN_CH_SEMI, "Expect ';' after variable declaration.", &c->ykes.parser);
}

static void synchronize(parser *parser)
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

static void statement(compiler *c)
{

    if (match(TOKEN_PRINT, &c->ykes.parser))
        print_statement(c);
    else if (match(TOKEN_OP_REM, &c->ykes.parser))
        rm_statement(c);
    else if (match(TOKEN_IF, &c->ykes.parser))
        if_statement(c);
    else if (match(TOKEN_WHILE, &c->ykes.parser))
        while_statement(c);
    else if (match(TOKEN_FOR, &c->ykes.parser))
        for_statement(c);
    else if (match(TOKEN_SWITCH, &c->ykes.parser))
        switch_statement(c);
    else if (match(TOKEN_CH_LCURL, &c->ykes.parser))
        block(c);
    else if (match(TOKEN_EACH, &c->ykes.parser))
        each_statement(c);
    else if (match(TOKEN_RETURN, &c->ykes.parser))
        return_statement(c);
    else if (is_comment(&c->ykes.parser))
        advance_compiler(&c->ykes.parser);
    else
        default_expression(c);
}

static void rm_statement(compiler *c)
{
    consume(TOKEN_CH_LPAREN, "Expect `(` prior to rm expression.", &c->ykes.parser);
    if (match(TOKEN_ID, &c->ykes.parser))
        ;

    arena ar = parse_id(c);
    uint8_t get;
    int arg = resolve_local(c, &ar);

    if (arg != -1)
        get = OP_GET_LOCAL;
    else if ((arg = resolve_upvalue(c, &ar)) != -1)
        get = OP_GET_UPVALUE;
    else
    {
        arg = add_constant(&c->ykes.func->ch, OBJ(ar));
        get = OP_GET_GLOBAL;
    }
    emit_bytes(c, get, (uint8_t)arg);
    emit_byte(c, OP_RM);
    consume(TOKEN_CH_RPAREN, "Expect `)` after rm statement", &c->ykes.parser);
    consume(TOKEN_CH_SEMI, "Expect `;` at end of statement", &c->ykes.parser);
}

static inline bool is_comment(parser *parser)
{
    return check(TOKEN_LINE_COMMENT, parser) || check(TOKEN_NLINE_COMMENT, parser);
}

static void for_statement(compiler *c)
{
    begin_scope(c);
    consume(TOKEN_CH_LPAREN, "Expect `(` after to 'for'.", &c->ykes.parser);
    if (!match(TOKEN_CH_SEMI, &c->ykes.parser))
    {
        if (match(TOKEN_VAR, &c->ykes.parser))
            var_dec(c);
        else
            id(c);
    }

    int start = c->ykes.func->ch.op_codes.count;
    int exit = -1;
    if (!match(TOKEN_CH_SEMI, &c->ykes.parser))
    {
        expression(c);
        consume(TOKEN_CH_SEMI, "Expect `;` after 'for' condition.", &c->ykes.parser);
        exit = emit_jump(c, OP_JMPF);
        emit_byte(c, OP_POP);
    }
    if (!match(TOKEN_CH_RPAREN, &c->ykes.parser))
    {
        int body_jump = emit_jump(c, OP_JMP);
        int inc_start = c->ykes.func->ch.op_codes.count;

        expression(c);
        emit_byte(c, OP_POP);
        consume(TOKEN_CH_RPAREN, "Expect `)` after 'for' statement.", &c->ykes.parser);

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

static void while_statement(compiler *c)
{
    int start = c->ykes.func->ch.op_codes.count;

    consume(TOKEN_CH_LPAREN, "Expect `(` after 'while'.", &c->ykes.parser);
    expression(c);
    consume(TOKEN_CH_RPAREN, "Expect `)` after 'while' condition.", &c->ykes.parser);

    int exit_jmp = emit_jump(c, OP_JMPF);
    emit_byte(c, OP_POP);

    statement(c);

    // If falsey, loop
    emit_loop(c, start);

    // If true, exit
    patch_jump(c, exit_jmp);
    emit_byte(c, OP_POP);
}
static void each_statement(compiler *c)
{
    emit_byte(c, OP_RESET_ARGC);

    consume(TOKEN_CH_LPAREN, "Expect `(` prior to each expression.", &c->ykes.parser);

    arena ar;
    int glob = 0;
    uint8_t set = 0;

    if (match(TOKEN_VAR, &c->ykes.parser))
    {
        consume(TOKEN_ID, "Expect identifier after var keyword", &c->ykes.parser);
        ar = parse_id(c);
        glob = parse_var(c, ar);
    }
    else
        error("Expect variable declaration at start of each expression", &c->ykes.parser);

    if (glob != -1)
        set = OP_SET_GLOBAL;
    else if ((glob = resolve_upvalue(c, &ar)) != -1)
        set = OP_SET_UPVALUE;
    else
    {
        glob = resolve_local(c, &ar);
        set = OP_SET_LOCAL;
    }
    consume(TOKEN_CH_COLON, "Expect `:` between variable declaration and identifier", &c->ykes.parser);

    int start = c->ykes.func->ch.op_codes.count;

    id(c);
    emit_byte(c, OP_EACH_ACCESS);
    int exit = emit_jump(c, OP_JMP_NIL);

    emit_bytes(c, set, (uint8_t)glob);

    consume(TOKEN_CH_RPAREN, "Expect `)` following an each expression.", &c->ykes.parser);

    statement(c);
    emit_loop(c, start);

    patch_jump(c, exit);
}

static void consume_if(compiler *c)
{
    consume(TOKEN_CH_LPAREN, "Expect `(` after an 'if'.", &c->ykes.parser);
    expression(c);
    consume(TOKEN_CH_RPAREN, "Expect `)` after an 'if' condtion.", &c->ykes.parser);
}
static void consume_elif(compiler *c)
{
    consume(TOKEN_CH_LPAREN, "Expect `(` after an 'elif'.", &c->ykes.parser);
    expression(c);
    consume(TOKEN_CH_RPAREN, "Expect `)` after an 'elif' condtion.", &c->ykes.parser);
}

static arena consume_switch(compiler *c)
{
    consume(TOKEN_CH_LPAREN, "Expect `(` after a 'switch'.", &c->ykes.parser);
    arena args = get_id(c);
    consume(TOKEN_CH_RPAREN, "Expect `)` after a 'switch' condtion.", &c->ykes.parser);
    return args;
}

static void switch_statement(compiler *c)
{
    arena args = consume_switch(c);
    bool expect = match(TOKEN_CH_LCURL, &c->ykes.parser);

    case_statement(c, args);

    if (match(TOKEN_DEFAULT, &c->ykes.parser))
    {
        consume(TOKEN_CH_COLON, "Expect `:` prior to case body.", &c->ykes.parser);
        statement(c);
    }
    if (expect)
        consume(TOKEN_CH_RCURL, "Expect `}` after switch body", &c->ykes.parser);

    element el = OBJ(c->ykes.func->ch.cases);
    push_int(&el, c->ykes.func->ch.op_codes.count);
}

static void case_statement(compiler *c, arena args)
{
    uint8_t get = args.listof.Ints[1];
    uint8_t arg = args.listof.Ints[0];

    while (match(TOKEN_CASE, &c->ykes.parser))
    {
        expression(c);
        consume(TOKEN_CH_COLON, "Expect `:` prior to case body.", &c->ykes.parser);
        emit_bytes(c, get, (uint8_t)arg);
        emit_byte(c, OP_SEQ);

        int tr = emit_jump_long(c, OP_JMPC);
        int begin = c->ykes.func->ch.op_codes.count;
        emit_byte(c, OP_POP);
        statement(c);
        emit_byte(c, OP_JMPL);
        emit_bytes(
            c,
            (c->ykes.func->ch.cases.count >> 8) & 0xFF,
            (c->ykes.func->ch.cases.count & 0xFF));
        patch_jump_long(c, begin, tr);
    }

    FREE_ARENA(&args);
}

static void if_statement(compiler *c)
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

    if (match(TOKEN_ELSE, &c->ykes.parser))
        statement(c);

    element el = OBJ(c->ykes.func->ch.cases);
    push_int(&el, c->ykes.func->ch.op_codes.count);
    patch_jump(c, exit);
}

static void elif_statement(compiler *c)
{
    while (match(TOKEN_ELIF, &c->ykes.parser))
    {
        consume_elif(c);
        int tr = emit_jump_long(c, OP_JMPC);
        int begin = c->ykes.func->ch.op_codes.count;
        emit_byte(c, OP_POP);
        statement(c);
        emit_byte(c, OP_JMPL);
        emit_bytes(
            c,
            (c->ykes.func->ch.cases.count >> 8) & 0xFF,
            (c->ykes.func->ch.cases.count & 0xFF));
        patch_jump_long(c, begin, tr);
    }
}

static void ternary_statement(compiler *c)
{
    int exit = emit_jump(c, OP_JMPF);
    emit_byte(c, OP_POP);
    expression(c);
    consume(TOKEN_CH_COLON, "Expect `:` between ternary expressions.", &c->ykes.parser);
    int tr = emit_jump(c, OP_JMP);
    patch_jump(c, exit);
    emit_byte(c, OP_POP);
    expression(c);
    patch_jump(c, tr);
}
static void null_coalescing_statement(compiler *c)
{
    int exit = emit_jump(c, OP_JMP_NOT_NIL);
    emit_byte(c, OP_POP);
    expression(c);
    patch_jump(c, exit);
    // emit_byte(c, OP_POP);
}

static void return_statement(compiler *c)
{
    if (c->meta.type == SCRIPT)
        error("ERROR: Unable to return from top of script.", &c->ykes.parser);

    else if (match(TOKEN_CH_SEMI, &c->ykes.parser))
        emit_return(c);
    else
    {
        if (c->meta.type == INIT)
            error("ERROR: Unable to return value from initializer.", &c->ykes.parser);
        expression(c);
        consume(TOKEN_CH_SEMI, "ERROR: Expect semi colon after return statement.", &c->ykes.parser);
        emit_byte(c, OP_RETURN);
    }
}

static void patch_jump_long(compiler *c, int count, int offset)
{

    int j1 = count - offset - 4;
    int j2 = (c->ykes.func->ch.op_codes.count) - offset - 4;

    if (j1 >= INT16_MAX)
        error("ERROR: To great a distance ", &c->ykes.parser);

    c->ykes.func->ch.op_codes.listof.Bytes[offset] = (uint8_t)((j2 >> 8) & 0xFF);
    c->ykes.func->ch.op_codes.listof.Bytes[offset + 1] = (uint8_t)(j2 & 0xFF);

    c->ykes.func->ch.op_codes.listof.Bytes[offset + 2] = (uint8_t)((j1 >> 8) & 0xFF);
    c->ykes.func->ch.op_codes.listof.Bytes[offset + 3] = (uint8_t)(j1 & 0xFF);
}

static void patch_jump(compiler *c, int offset)
{

    int jump = c->ykes.func->ch.op_codes.count - offset - 2;

    if (jump >= INT16_MAX)
        error("ERROR: To great a distance ", &c->ykes.parser);

    c->ykes.func->ch.op_codes.listof.Bytes[offset] = (uint8_t)((jump >> 8) & 0xFF);
    c->ykes.func->ch.op_codes.listof.Bytes[offset + 1] = (uint8_t)(jump & 0xFF);
}

static void emit_loop(compiler *c, int byte)
{
    emit_byte(c, OP_LOOP);

    int offset = c->ykes.func->ch.op_codes.count - byte + 2;

    if (offset > UINT16_MAX)
        error("ERROR: big boi loop", &c->ykes.parser);

    emit_bytes(c, (offset >> 8) & 0xFF, offset & 0xFF);
}
static int emit_jump_long(compiler *c, int byte)
{
    emit_byte(c, byte);
    emit_bytes(c, 0xFF, 0xFF);
    emit_bytes(c, 0xFF, 0xFF);

    return c->ykes.func->ch.op_codes.count - 4;
}
static int emit_jump(compiler *c, int byte)
{
    emit_byte(c, byte);
    emit_bytes(c, 0xFF, 0xFF);

    return c->ykes.func->ch.op_codes.count - 2;
}

static void default_expression(compiler *c)
{
    expression(c);
    consume(TOKEN_CH_SEMI, "Expect `;` after expression.", &c->ykes.parser);
    emit_byte(c, OP_POP);
}

static void print_statement(compiler *c)
{
    consume(TOKEN_CH_LPAREN, "Expect `(` prior to print expression", &c->ykes.parser);
    do
    {
        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);
        if (check(TOKEN_CH_COMMA, &c->ykes.parser))
            emit_byte(c, OP_PRINT);
    } while (match(TOKEN_CH_COMMA, &c->ykes.parser));

    emit_byte(c, OP_PRINT);

    consume(TOKEN_CH_RPAREN, "Expect `)` after print expression", &c->ykes.parser);
    consume(TOKEN_CH_SEMI, "Expect ';' after value.", &c->ykes.parser);
}

static void begin_scope(compiler *c)
{
    c->count.scope++;
}
static void end_scope(compiler *c)
{

    c->count.scope--;
    if (c->count.local > 0 && (c->stack.local[c->count.local - 1].depth > c->count.scope))
        emit_bytes(c, OP_POPN, add_constant(&c->ykes.func->ch, OBJ(Int(c->count.local - 1))));

    while (c->count.local > 0 && (c->stack.local[c->count.local - 1].depth > c->count.scope))
    {
        if (c->stack.local[c->count.local - 1].captured)
            emit_byte(c, OP_CLOSE_UPVAL);
        else
            ARENA_FREE(&c->stack.local[c->count.local - 1].name);
        --c->count.local;
    }
}

static void parse_block(compiler *c)
{
    while (!check(TOKEN_CH_RCURL, &c->ykes.parser) && !check(TOKEN_EOF, &c->ykes.parser))
        declaration(c);
    consume(TOKEN_CH_RCURL, "Expect `}` after block statement", &c->ykes.parser);
}
static void block(compiler *c)
{
    begin_scope(c);
    parse_block(c);
    end_scope(c);
}

static bool match(int t, parser *parser)
{
    if (!check(t, parser))
        return false;
    advance_compiler(parser);
    return true;
}
static bool check(int t, parser *parser)
{
    return parser->cur.type == t;
}

static void expression(compiler *c)
{
    parse_precedence(PREC_ASSIGNMENT, c);
}
static void grouping(compiler *c)
{
    expression(c);
    consume(TOKEN_CH_RPAREN, "Expect `)` after expression", &c->ykes.parser);
}
static PRule *get_rule(int t)
{
    return &rules[t];
}

static void parse_precedence(Precedence prec, compiler *c)
{
    advance_compiler(&c->ykes.parser);
    parse_fn prefix_rule = get_rule(c->ykes.parser.pre.type)->prefix;

    if (!prefix_rule)
    {
        error("ERROR: Expect expression.", &c->ykes.parser);
        return;
    }

    prefix_rule(c);

    while (prec <= get_rule(c->ykes.parser.cur.type)->prec)
    {
        advance_compiler(&c->ykes.parser);
        parse_fn infix = get_rule(c->ykes.parser.pre.type)->infix;
        infix(c);
    }
}

static void _and(compiler *c)
{
    int end = emit_jump(c, OP_JMPF);

    emit_byte(c, OP_POP);
    parse_precedence(PREC_AND, c);

    patch_jump(c, end);
}
static void _or(compiler *c)
{
    int else_jmp = emit_jump(c, OP_JMPT);

    emit_byte(c, OP_POP);
    parse_precedence(PREC_OR, c);

    patch_jump(c, else_jmp);
}

static void binary(compiler *c)
{
    int t = c->ykes.parser.pre.type;

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
static void unary(compiler *c)
{
    int op = c->ykes.parser.pre.type;

    parse_precedence(PREC_UNARY, c);

    switch (op)
    {

    case TOKEN_OP_SUB:
    case TOKEN_OP_BANG:
        emit_byte(c, OP_NEG);
        break;
    default:
        return;
    }
}

static void current_err(const char *err, parser *parser)
{
    error_at(&parser->cur, parser, err);
}
static void error(const char *err, parser *parser)
{
    error_at(&parser->pre, parser, err);
}
static void error_at(Token toke, parser *parser, const char *err)
{
    if (parser->panic)
        return;
    parser->panic = true;
    parser->err = true;

    fprintf(stderr, "[file: %s, line: %d] Error", parser->current_file, toke->line - 1);

    if (toke->type == TOKEN_EOF)
        fprintf(stderr, " at end");
    else if (toke->type != TOKEN_ERR)
        fprintf(stderr, " at '%.*s'", toke->size, toke->start);

    fprintf(stderr, ": %s\n", err);
}

static void emit_return(compiler *c)
{
    if (c->meta.type == INIT)
        emit_bytes(c, OP_GET_LOCAL, 0);
    else
        emit_byte(c, OP_NULL);
    emit_byte(c, OP_RETURN);
}
static void emit_byte(compiler *c, uint8_t byte)
{
    write_chunk(&c->ykes.func->ch, byte, c->ykes.parser.pre.line);
}
static void emit_bytes(compiler *c, uint8_t b1, uint8_t b2)
{
    write_chunk(&c->ykes.func->ch, b1, c->ykes.parser.pre.line);
    write_chunk(&c->ykes.func->ch, b2, c->ykes.parser.pre.line);
}
static void emit_constant(compiler *c, arena ar)
{
    emit_bytes(
        c, OP_CONSTANT,
        add_constant(&c->ykes.func->ch, OBJ(ar)));
}

static void pi(compiler *c)
{
    emit_constant(c, Double(M_PI));
}

static void euler(compiler *c)
{
    emit_constant(c, Double(M_E));
}

static void dval(compiler *c)
{
    double val = strtod(c->ykes.parser.pre.start, NULL);
    emit_constant(c, Double(val));
}
static void ival(compiler *c)
{
    emit_constant(c, Int(atoi(c->ykes.parser.pre.start)));
}
static void llint(compiler *c)
{
    emit_constant(c, Long(atoll(c->ykes.parser.pre.start)));
}
static void ch(compiler *c)
{
    emit_constant(c, Char(*++c->ykes.parser.pre.start));
}

static void boolean(compiler *c)
{
    if (*c->ykes.parser.pre.start == 'n')
        emit_bytes(c, OP_CONSTANT, add_constant(&c->ykes.func->ch, CLASS(NULL)));
    else
        emit_constant(c, Bool(*c->ykes.parser.pre.start == 't' ? true : false));
}
static const char *parse_string(compiler *c)
{
    char *ch = (char *)++c->ykes.parser.pre.start;
    ch[c->ykes.parser.pre.size - 2] = '\0';
    return ch;
}
static void cstr(compiler *c)
{
    emit_constant(c, CString(parse_string(c)));
}
static void string(compiler *c)
{
    consume(TOKEN_CH_LPAREN, "Expect `(` prior to string declaration.", &c->ykes.parser);
    consume(TOKEN_STR, "Expect string declaration", &c->ykes.parser);
    emit_constant(c, String(parse_string(c)));
    consume(TOKEN_CH_RPAREN, "Expect `)` after string declaration.", &c->ykes.parser);
}

static void array_alloc(compiler *c)
{
    consume(TOKEN_CH_LPAREN, "Expect `(` prior to allocation.", &c->ykes.parser);

    if (match(TOKEN_INT, &c->ykes.parser))
        emit_constant(c, GROW_ARENA(NULL, atoi(c->ykes.parser.pre.start), ARENA_INTS));
    else if (match(TOKEN_ID, &c->ykes.parser))
    {
        id(c);
        emit_byte(c, OP_CPY_ARRAY);
    }
    else if (match(TOKEN_CH_LSQUARE, &c->ykes.parser))
        array(c);
    else
        error("ERROR: Invalid expression inside of array allocation", &c->ykes.parser);

    consume(TOKEN_CH_RPAREN, "Expect `)` after allocation.", &c->ykes.parser);
}
static void vector_alloc(compiler *c)
{
    arena *ar = NULL;
    consume(TOKEN_CH_LPAREN, "Expect `(` prior to vector allocation", &c->ykes.parser);
    if (match(TOKEN_CH_RPAREN, &c->ykes.parser))
    {
        ar = GROW_VECTOR(NULL, STACK_SIZE);
        emit_bytes(c, OP_CONSTANT, add_constant(&c->ykes.func->ch, VECT(ar)));
        return;
    }

    if (match(TOKEN_INT, &c->ykes.parser))
    {
        ar = GROW_VECTOR(NULL, atoi(c->ykes.parser.pre.start));
        emit_bytes(
            c, OP_CONSTANT,
            add_constant(&c->ykes.func->ch, VECT(ar)));
    }
    else
        error("ERROR: Invalid expression inside of vector allocation", &c->ykes.parser);

    consume(TOKEN_CH_RPAREN, "Expect `)` after vector allocation.", &c->ykes.parser);
}

static void stack_alloc(compiler *c)
{
    stack *s = NULL;
    consume(TOKEN_CH_LPAREN, "Expect `(` prior to stack allocation", &c->ykes.parser);

    if (match(TOKEN_CH_RPAREN, &c->ykes.parser))
    {
        s = GROW_STACK(NULL, STACK_SIZE);
        emit_bytes(c, OP_CONSTANT, add_constant(&c->ykes.func->ch, STK(s)));
        return;
    }

    if (match(TOKEN_INT, &c->ykes.parser))
    {
        s = _stack(atoi(c->ykes.parser.pre.start));
        emit_bytes(
            c, OP_CONSTANT,
            add_constant(&c->ykes.func->ch, STK(s)));
    }
    else
        error("ERROR: Invalid expression inside of stack allocation", &c->ykes.parser);

    consume(TOKEN_CH_RPAREN, "Expect `)` after stack allocation", &c->ykes.parser);
}

static void _table(compiler *c)
{
    table *t = NULL;
    consume(TOKEN_CH_LPAREN, "Expect `(` prior to table allocation", &c->ykes.parser);

    if (match(TOKEN_CH_RPAREN, &c->ykes.parser))
    {
        t = GROW_TABLE(NULL, STACK_SIZE);
        emit_bytes(c, OP_CONSTANT, add_constant(&c->ykes.func->ch, TABLE(t)));
        return;
    }
    else
    {
        expression(c);
        emit_byte(c, OP_ALLOC_TABLE);
        consume(TOKEN_CH_RPAREN, "Expect `)` after table declaration", &c->ykes.parser);
    }
}

static int resolve_native(compiler *c, arena *ar)
{

    element el = find_entry(&c->ykes.base->lookup.native, ar);

    if (el.type == ARENA && el._arena.type == ARENA_INT)
        return el._arena.as.Int;

    return -1;
}
static void parse_native_var_arg(compiler *c)
{

    char *ch = (char *)c->ykes.parser.pre.start;
    ch[c->ykes.parser.pre.size] = '\0';

    arena ar = Var(ch);
    int arg = resolve_native(c, &ar);
    emit_bytes(c, OP_GET_NATIVE, (uint8_t)arg);
    consume(TOKEN_CH_LPAREN, "Expect `(` prior to function call", &c->ykes.parser);
    call(c);
}

static arena parse_func_id(compiler *c)
{
    char *ch = (char *)c->ykes.parser.pre.start;
    ch[c->ykes.parser.pre.size] = '\0';
    return Var(ch);
}

static arena parse_id(compiler *c)
{
    char *ch = (char *)c->ykes.parser.pre.start;
    ch[c->ykes.parser.pre.size] = '\0';
    return Var(ch);
}

static arena get_id(compiler *c)
{
    bool pre_inc = (c->ykes.parser.pre.type == TOKEN_OP_INC);
    bool pre_dec = (c->ykes.parser.pre.type == TOKEN_OP_DEC);

    if (match(TOKEN_ID, &c->ykes.parser))
        ;

    arena ar = parse_id(c);

    uint8_t get, set;

    arena args = GROW_ARENA(NULL, 3 * sizeof(int), ARENA_INTS);
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
        arg = add_constant(&c->ykes.func->ch, OBJ(ar));
        get = OP_GET_GLOBAL;
        set = OP_SET_GLOBAL;
    }
    args.listof.Ints[0] = arg;
    args.listof.Ints[1] = get;

    if (pre_inc)
        emit_bytes(c, get == OP_GET_LOCAL ? OP_INC_LOC : OP_INC_GLO, (uint8_t)arg);
    else if (pre_dec)
        emit_bytes(c, get == OP_GET_LOCAL ? OP_DEC_LOC : OP_DEC_GLO, (uint8_t)arg);
    else if (match(TOKEN_OP_DEC, &c->ykes.parser))
        emit_bytes(c, get == OP_GET_LOCAL ? OP_DEC_LOC : OP_DEC_GLO, (uint8_t)arg);
    else if (match(TOKEN_OP_INC, &c->ykes.parser))
        emit_bytes(c, get == OP_GET_LOCAL ? OP_INC_LOC : OP_INC_GLO, (uint8_t)arg);
    else if (match(TOKEN_OP_ASSIGN, &c->ykes.parser))
    {
        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);
        emit_bytes(c, set, (uint8_t)arg);
    }
    else
        emit_bytes(c, get, (uint8_t)arg);
    return args;
}

static int resolve_call(compiler *c, arena *ar)
{

    element el = find_entry(&c->ykes.base->lookup.call, ar);

    if (el.type == ARENA && el._arena.type == ARENA_INT)
        return el._arena.as.Int;

    return -1;
}

static int resolve_instance(compiler *c, arena ar)
{
    element el = find_entry(&c->ykes.base->lookup.class, &ar);

    if (el.type == ARENA && el._arena.type == ARENA_INT)
        return el._arena.as.Int;

    return -1;
}

static void push_array_val(compiler *c)
{
    consume(TOKEN_CH_LPAREN, "Expect `(` after push.", &c->ykes.parser);
    expression(c);
    emit_byte(c, OP_PUSH_ARRAY_VAL);
    consume(TOKEN_CH_RPAREN, "Expect `)` after push expression.", &c->ykes.parser);
}
static void pop_array_val(compiler *c)
{
    consume(TOKEN_CH_LPAREN, "Expect `(` after push.", &c->ykes.parser);
    emit_byte(c, OP_POP__ARRAY_VAL);
    consume(TOKEN_CH_RPAREN, "Expect `)` after push expression.", &c->ykes.parser);
}

static void dot(compiler *c)
{
    match(TOKEN_ID, &c->ykes.parser);

    arena ar = parse_id(c);

    if (ar.as.hash == c->ykes.base->_hash_ref.len.as.hash)
    {
        emit_byte(c, OP_LEN);
        return;
    }
    if (ar.as.hash == c->ykes.base->_hash_ref.push.as.hash)
    {
        push_array_val(c);
        int exit = emit_jump(c, OP_JMPT);
        emit_byte(c, OP_POP);
        emit_bytes(c, c->array.set, c->array.index);

        int falsey = emit_jump(c, OP_JMP);
        patch_jump(c, exit);
        emit_byte(c, OP_POP);
        patch_jump(c, falsey);
        return;
    }

    if (ar.as.hash == c->ykes.base->_hash_ref.pop.as.hash)
    {
        pop_array_val(c);
        emit_bytes(c, c->array.set, c->array.index);
        emit_byte(c, OP_PUSH);
        return;
    }

    if (match(TOKEN_OP_ASSIGN, &c->ykes.parser))
    {
        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);
        emit_bytes(c, OP_SET_PROP, (uint8_t)add_constant(&c->ykes.func->ch, OBJ(ar)));
    }
    else if (match(TOKEN_ADD_ASSIGN, &c->ykes.parser))
    {

        emit_byte(c, OP_PUSH_TOP);
        emit_bytes(c, OP_GET_PROP, (uint8_t)add_constant(&c->ykes.func->ch, OBJ(ar)));
        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);
        emit_byte(c, OP_ADD);
        emit_bytes(c, OP_SET_PROP, (uint8_t)add_constant(&c->ykes.func->ch, OBJ(ar)));
    }
    else if (match(TOKEN_SUB_ASSIGN, &c->ykes.parser))
    {

        emit_byte(c, OP_PUSH_TOP);
        emit_bytes(c, OP_GET_PROP, (uint8_t)add_constant(&c->ykes.func->ch, OBJ(ar)));
        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);
        emit_bytes(c, OP_SET_PROP, (uint8_t)add_constant(&c->ykes.func->ch, OBJ(ar)));
    }
    else if (match(TOKEN_MUL_ASSIGN, &c->ykes.parser))
    {

        emit_byte(c, OP_PUSH_TOP);
        emit_bytes(c, OP_GET_PROP, (uint8_t)add_constant(&c->ykes.func->ch, OBJ(ar)));
        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);
        emit_byte(c, OP_MUL);
        emit_bytes(c, OP_SET_PROP, (uint8_t)add_constant(&c->ykes.func->ch, OBJ(ar)));
    }
    else if (match(TOKEN_DIV_ASSIGN, &c->ykes.parser))
    {

        emit_byte(c, OP_PUSH_TOP);
        emit_bytes(c, OP_GET_PROP, (uint8_t)add_constant(&c->ykes.func->ch, OBJ(ar)));
        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);
        emit_byte(c, OP_DIV);
        emit_bytes(c, OP_SET_PROP, (uint8_t)add_constant(&c->ykes.func->ch, OBJ(ar)));
    }
    else if (match(TOKEN_MOD_ASSIGN, &c->ykes.parser))
    {
        emit_byte(c, OP_PUSH_TOP);
        emit_bytes(c, OP_GET_PROP, (uint8_t)add_constant(&c->ykes.func->ch, OBJ(ar)));

        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);
        emit_byte(c, OP_MOD);
        emit_bytes(c, OP_SET_PROP, (uint8_t)add_constant(&c->ykes.func->ch, OBJ(ar)));
    }
    else if (match(TOKEN_AND_ASSIGN, &c->ykes.parser))
    {

        emit_byte(c, OP_PUSH_TOP);
        emit_bytes(c, OP_GET_PROP, (uint8_t)add_constant(&c->ykes.func->ch, OBJ(ar)));
        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);
        emit_bytes(c, OP_SET_PROP, (uint8_t)add_constant(&c->ykes.func->ch, OBJ(ar)));
    }
    else if (match(TOKEN_OR__ASSIGN, &c->ykes.parser))
    {
        emit_byte(c, OP_PUSH_TOP);
        emit_bytes(c, OP_GET_PROP, (uint8_t)add_constant(&c->ykes.func->ch, OBJ(ar)));

        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);

        emit_bytes(c, OP_SET_PROP, (uint8_t)add_constant(&c->ykes.func->ch, OBJ(ar)));
    }
    else
    {
        emit_bytes(c, OP_GET_PROP, add_constant(&c->ykes.func->ch, OBJ(ar)));
    }
}

static void int_array(compiler *c)
{
    element el = OBJ(Ints(NULL, 0));

    do
    {
        if (match(TOKEN_INT, &c->ykes.parser))
            push_int(&el, atoi(c->ykes.parser.pre.start));
        else
        {
            error("Unexpected type in array declaration", &c->ykes.parser);
            return;
        }
    } while (match(TOKEN_CH_COMMA, &c->ykes.parser));

    emit_bytes(c, OP_CONSTANT, add_constant(&c->ykes.func->ch, el));
}
static void double_array(compiler *c)
{
    element el = OBJ(Doubles(NULL, 0));

    do
    {
        if (match(TOKEN_DOUBLE, &c->ykes.parser))
            push_double(&el, strtod(c->ykes.parser.pre.start, NULL));
        else
        {
            error("Unexpected type in array declaration", &c->ykes.parser);
            return;
        }
    } while (match(TOKEN_CH_COMMA, &c->ykes.parser));

    emit_bytes(c, OP_CONSTANT, add_constant(&c->ykes.func->ch, el));
}
static void string_array(compiler *c)
{
    element el = OBJ(Strings());

    do
    {
        if (match(TOKEN_STR, &c->ykes.parser))
            push_string(&el, parse_string(c));
        else
        {
            error("Unexpected type in array declaration", &c->ykes.parser);
            return;
        }
    } while (match(TOKEN_CH_COMMA, &c->ykes.parser));

    emit_bytes(c, OP_CONSTANT, add_constant(&c->ykes.func->ch, el));
}
static void long_array(compiler *c)
{
    element el = OBJ(Longs(NULL, 0));
    do
    {
        if (match(TOKEN_LLINT, &c->ykes.parser))
            push_long(&el, atoll(c->ykes.parser.pre.start));
        else
        {
            error("Unexpected type in array declaration", &c->ykes.parser);
            return;
        }
    } while (match(TOKEN_CH_COMMA, &c->ykes.parser));

    emit_bytes(c, OP_CONSTANT, add_constant(&c->ykes.func->ch, el));
}

static void array(compiler *c)
{

    switch (c->ykes.parser.cur.type)
    {
    case TOKEN_INT:
        int_array(c);
        break;
    case TOKEN_STR:
        string_array(c);
        break;
    case TOKEN_DOUBLE:
        double_array(c);
        break;
    case TOKEN_LLINT:
        long_array(c);
        break;
    default:
        error("Unexpected type in array declaration", &c->ykes.parser);
    }

    consume(TOKEN_CH_RSQUARE, "Expect closing brace after array declaration.", &c->ykes.parser);
}

static void _access(compiler *c)
{

    expression(c);
    consume(TOKEN_CH_RSQUARE, "Expect closing brace after array access.", &c->ykes.parser);

    if (match(TOKEN_OP_ASSIGN, &c->ykes.parser))
    {
        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);
        emit_byte(c, OP_SET_ACCESS);
    }
    else
        emit_byte(c, OP_GET_ACCESS);
}

static void _this(compiler *c)
{
    if (!c->ykes.class_compiler)
    {
        error("ERROR: can't use `this` keyword outside of a class body.", &c->ykes.parser);
        return;
    }

    int arg = resolve_instance(c, c->ykes.class_compiler->instance_name);

    emit_bytes(
        c, OP_GET_CLASS,
        arg);
}

static void id(compiler *c)
{
    bool pre_inc = (c->ykes.parser.pre.type == TOKEN_OP_INC);
    bool pre_dec = (c->ykes.parser.pre.type == TOKEN_OP_DEC);

    match(TOKEN_ID, &c->ykes.parser);

    arena ar = parse_func_id(c);
    uint8_t get, set;
    int arg = resolve_call(c, &ar);

    if (arg != -1)
    {
        emit_bytes(c, OP_GET_CLOSURE, (uint8_t)arg);
        return;
    }

    if ((arg = resolve_instance(c, ar)) != -1)
    {

        if (c->ykes.base->stack.instance[arg]->init)
        {
            emit_bytes(c, OP_CONSTANT, (uint8_t)add_constant(&c->ykes.func->ch, CLOSURE(c->ykes.base->stack.instance[arg]->init)));
            match(TOKEN_CH_LPAREN, &c->ykes.parser);
            call(c);
        }
        emit_bytes(c, OP_GET_CLASS, (uint8_t)arg);
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

    else
    {
        arg = add_constant(&c->ykes.func->ch, OBJ(ar));
        get = OP_GET_GLOBAL;
        set = OP_SET_GLOBAL;
    }

    if (pre_inc)
        emit_bytes(c, get == OP_GET_LOCAL ? OP_INC_LOC : OP_INC_GLO, (uint8_t)arg);
    else if (pre_dec)
        emit_bytes(c, get == OP_GET_LOCAL ? OP_DEC_LOC : OP_DEC_GLO, (uint8_t)arg);
    else if (match(TOKEN_OP_DEC, &c->ykes.parser))
        emit_bytes(c, get == OP_GET_LOCAL ? OP_DEC_LOC : OP_DEC_GLO, (uint8_t)arg);
    else if (match(TOKEN_OP_INC, &c->ykes.parser))
        emit_bytes(c, get == OP_GET_LOCAL ? OP_INC_LOC : OP_INC_GLO, (uint8_t)arg);
    else if (match(TOKEN_OP_ASSIGN, &c->ykes.parser))
    {
        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);
        emit_bytes(c, set, (uint8_t)arg);
    }
    else if (match(TOKEN_ADD_ASSIGN, &c->ykes.parser))
    {
        emit_bytes(c, get, (uint8_t)arg);
        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);
        emit_byte(c, OP_ADD);
        emit_bytes(c, set, (uint8_t)arg);
    }
    else if (match(TOKEN_SUB_ASSIGN, &c->ykes.parser))
    {
        emit_bytes(c, get, (uint8_t)arg);
        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);
        emit_byte(c, OP_SUB);
        emit_bytes(c, set, (uint8_t)arg);
    }
    else if (match(TOKEN_MUL_ASSIGN, &c->ykes.parser))
    {

        emit_bytes(c, get, (uint8_t)arg);
        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);
        emit_byte(c, OP_MUL);
        emit_bytes(c, set, (uint8_t)arg);
    }
    else if (match(TOKEN_DIV_ASSIGN, &c->ykes.parser))
    {
        emit_bytes(c, get, (uint8_t)arg);
        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);
        emit_byte(c, OP_DIV);
        emit_bytes(c, set, (uint8_t)arg);
    }
    else if (match(TOKEN_MOD_ASSIGN, &c->ykes.parser))
    {
        emit_bytes(c, get, (uint8_t)arg);
        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);
        emit_byte(c, OP_MOD);
        emit_bytes(c, set, (uint8_t)arg);
    }
    else if (match(TOKEN_AND_ASSIGN, &c->ykes.parser))
    {
        emit_bytes(c, get, (uint8_t)arg);
        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);
        emit_byte(c, OP_AND);
        emit_bytes(c, set, (uint8_t)arg);
    }
    else if (match(TOKEN_OR__ASSIGN, &c->ykes.parser))
    {
        emit_bytes(c, get, (uint8_t)arg);
        expression(c);
        if (match(TOKEN_CH_TERNARY, &c->ykes.parser))
            ternary_statement(c);
        else if (match(TOKEN_CH_NULL_COALESCING, &c->ykes.parser))
            null_coalescing_statement(c);
        emit_byte(c, OP_OR);
        emit_bytes(c, set, (uint8_t)arg);
    }
    else
    {
        emit_bytes(c, get, (uint8_t)arg);

        if (check(TOKEN_CH_DOT, &c->ykes.parser))
            c->array.set = set,
            c->array.index = arg;
    }
}

static int parse_var(compiler *c, arena ar)
{
    declare_var(c, ar);
    if (c->count.scope > 0)
        return -1;
    return add_constant(&c->ykes.func->ch, OBJ(ar));
}

static bool idcmp(arena a, arena b)
{
    if (a.as.len != b.as.len)
        return false;

    return a.as.hash == b.as.hash;
}

static int resolve_local(compiler *c, arena *name)
{
    for (int i = c->count.local - 1; i >= 0; i--)
        if (idcmp(*name, c->stack.local[i].name))
            return i;
    return -1;
}
static int resolve_upvalue(compiler *c, arena *name)
{

    if (!c->ykes.enclosing)
        return -1;

    int local = resolve_local(c->ykes.enclosing, name);

    if (local != -1)
    {
        c->ykes.enclosing->stack.local[local].captured = true;
        return add_upvalue(c, (uint8_t)local, true);
    }

    int upvalue = resolve_upvalue(c->ykes.enclosing, name);
    if (upvalue != -1)
        return add_upvalue(c, (uint8_t)upvalue, false);

    return -1;
}

static int add_upvalue(compiler *c, int index, bool islocal)
{
    int count = c->count.upvalue;

    for (int i = 0; i < count; i++)
    {
        upvalue *upval = &c->stack.upvalue[i];

        if (upval->index == index && upval->islocal == islocal)
            return i;
    }

    if (count > LOCAL_COUNT)
    {
        error("ERROR: To many closure variables in function.", &c->ykes.parser);
        return 0;
    }

    c->stack.upvalue[c->count.upvalue].islocal = islocal;
    c->stack.upvalue[c->count.upvalue].index = (uint8_t)index;
    c->count.upvalue++;
    return c->count.upvalue++;
}

static void declare_var(compiler *c, arena ar)
{
    if (c->count.scope == 0)
        return;

    for (int i = c->count.local - 1; i >= 0; i--)
    {
        local *_local = &c->stack.local[i];

        if (_local->depth != 0 && _local->depth < c->count.scope)
            break;

        else if (idcmp(ar, _local->name))
            error("ERROR: Duplicate variable identifiers in scope", &c->ykes.parser);
    }

    add_local(c, &ar);
}

static void add_local(compiler *c, arena *ar)
{
    if (c->count.local == LOCAL_COUNT)
    {
        error("ERROR: Too many local variables in function.", &c->ykes.parser);
        return;
    }
    c->stack.local[c->count.local].name = *ar;
    c->stack.local[c->count.local].depth = c->count.scope;
    c->stack.local[c->count.local].captured = false;
    c->count.local++;
}
static function *end_compile(compiler *a)
{
    function *f = a->ykes.func;

    emit_return(a);
#ifdef DEBUG_PRINT_CODE
    if (!a->ykes.parser.err)
        disassemble_chunk(
            &a->ykes.func->ch,
            a->ykes.func->name.as.String);
#endif
    if (a->ykes.enclosing)
    {

        parser tmp = a->ykes.parser;
        a = a->ykes.enclosing;
        a->ykes.parser = tmp;
    }

    return f;
}

function *compile(const char *src)
{
    compiler c;

    init_scanner(src);

    init_compiler(&c, NULL, SCRIPT, Var("SCRIPT"));

    c._hash_ref.init = String("init");
    c.ykes.base = &c;
    c.ykes.base->meta.cwd = NULL;
    c.ykes.base->lookup.call = GROW_TABLE(NULL, STACK_SIZE);
    c.ykes.base->lookup.class = GROW_TABLE(NULL, STACK_SIZE);
    c.ykes.base->lookup.include = GROW_TABLE(NULL, STACK_SIZE);
    c.ykes.base->_hash_ref.len = CString("len");
    c.ykes.base->_hash_ref.push = CString("push");
    c.ykes.base->_hash_ref.pop = CString("pop");

    c.ykes.parser.panic = false;
    c.ykes.parser.err = false;

    advance_compiler(&c.ykes.parser);

    while (!match(TOKEN_EOF, &c.ykes.parser))
        declaration(&c);
    consume(TOKEN_EOF, "Expect end of expression", &c.ykes.parser);

    function *f = end_compile(&c);

    FREE((c.ykes.base->lookup.call - 1));
    FREE((c.ykes.base->lookup.class - 1));
    FREE((c.ykes.base->_hash_ref.init.as.String));

    return c.ykes.parser.err ? NULL : f;
}
function *compile_path(const char *src, const char *path, const char *name)
{
    compiler c;

    init_scanner(src);

    init_compiler(&c, NULL, SCRIPT, Var("SCRIPT"));

    c.ykes.base = &c;
    c.ykes.base->meta.cwd = path;

    c.ykes.base->lookup.call = GROW_TABLE(NULL, STACK_SIZE);
    c.ykes.base->lookup.class = GROW_TABLE(NULL, STACK_SIZE);
    c.ykes.base->lookup.include = GROW_TABLE(NULL, STACK_SIZE);
    c.ykes.base->lookup.native = GROW_TABLE(NULL, STACK_SIZE);

    c.ykes.base->_hash_ref.init = String("init");
    c.ykes.base->_hash_ref.len = CString("len");
    c.ykes.base->_hash_ref.push = CString("push");
    c.ykes.base->_hash_ref.pop = CString("pop");

    c.ykes.parser.panic = false;
    c.ykes.parser.err = false;
    c.ykes.parser.current_file = name;

    write_table(c.ykes.base->lookup.native, CString("clock"), OBJ(Int(c.ykes.base->count.native++)));
    write_table(c.ykes.base->lookup.native, CString("square"), OBJ(Int(c.ykes.base->count.native++)));
    write_table(c.ykes.base->lookup.native, CString("prime"), OBJ(Int(c.ykes.base->count.native++)));
    write_table(c.ykes.base->lookup.native, CString("file"), OBJ(Int(c.ykes.base->count.native++)));

    advance_compiler(&c.ykes.parser);

    while (!match(TOKEN_EOF, &c.ykes.parser))
        declaration(&c);
    consume(TOKEN_EOF, "Expect end of expression", &c.ykes.parser);

    function *f = end_compile(&c);

    FREE((char *)(c.ykes.parser.current_file));
    FREE(((c.ykes.base->lookup.call - 1)));
    FREE(((c.ykes.base->lookup.native - 1)));
    FREE(((c.ykes.base->lookup.class - 1)));
    FREE(((c.ykes.base->_hash_ref.init.as.String)));

    return c.ykes.parser.err ? NULL : f;
}
