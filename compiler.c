#include "chunk.h"
#include "compiler.h"
#include "compiler_util.h"
#include "error.h"
#include "object_string.h"
#include "table.h"
#ifdef DEBUG_TRACE_EXECUTION
#include "debug.h"
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define UPPER(x) ((uint8_t)((x >> 8) & 0xFF))
#define LOWER(x) ((uint8_t)(x & 0xFF))

static void init_compiler(compiler *a, compiler *b, compiler_t type, _key name)
{

	local *_local     = NULL;
	a->enclosing      = NULL;
	a->base           = NULL;
	a->class_compiler = NULL;
	a->func           = NULL;
	a->lookup         = NULL;
	a->meta.cwd       = NULL;

	a->meta.type  = type;
	a->meta.flags = 0;

	a->count.local   = 0;
	a->count.scope   = 0;
	a->count.obj     = 0;
	a->count.upvalue = 0;

	a->array.index = 0;
	a->array.set   = 0;
	a->array.get   = 0;

	a->func = _function(name);

	if (b)
	{
		a->base           = b->base;
		a->lookup         = b->base->lookup;
		a->parser         = b->parser;
		a->enclosing      = b;
		a->count.scope    = b->count.scope;
		a->class_compiler = b->class_compiler;

		_local = &b->stack.local[b->count.local++];
	}
	else
		_local = &a->stack.local[a->count.local++];

	_local->depth    = 0;
	_local->captured = false;
}

static void consume(token_t t, const char *err, parser *parser)
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
	buffer       = ALLOC(fileSize + 1);

	if (!buffer)
	{
		fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
		exit(74);
	}
	size_t bytesRead  = fread(buffer, sizeof(char), fileSize, file);
	buffer[bytesRead] = '\0';

	fclose(file);
	return buffer;
}

static bool resolve_include(compiler *c, _key *ar)
{
	return find_entry(&c->base->lookup, ar).type == T_KEY;
}

static char *get_name(char *path)
{
	int   len = strlen(path) - 1;
	char *tmp = path + len;

	int count;
	for (count = 0; tmp[-1] != '/'; --tmp, count++)
		;

	char *file = NULL;
	file       = ALLOC((count + 1) * sizeof(char));

	strcpy(file, tmp);

	return file;
}
static void include_file(compiler *c)
{

#define SIZE(x, y) (strlen(x) + strlen(y) + 2)

	if (c->meta.type != COMPILER_TYPE_SCRIPT)
	{
		prev_error("Can only include files at top level.", &c->parser);
		exit(1);
	}

	char *remaining = NULL;

	consume(TOKEN_STR, "Expect file path.", &c->parser);
	_key inc = parse_string(c);

	if (resolve_include(c, &inc))
	{
		prev_error("Double include.", &c->parser);
		exit(1);
	}

	write_table(c->base->lookup, inc, KEY(inc));

	consume(
	    TOKEN_CH_SEMI, "Expect `;` at end of include statement.", &c->parser
	);
	remaining = (char *)c->parser.cur.start;

	char path[CWD_MAX] = {0};

	strcpy(path, (char *)c->base->meta.cwd);
	strcat(path, inc.val);

	char *file = read_file(path);

	char *result = NULL;
	result       = ALLOC(SIZE(file, remaining));

	strcpy(result, file);

	strcat(result, remaining);
	init_scanner(result);
	c->parser.cur = scan_token();

	c->parser.current_file = get_name(inc.val);

#undef SIZE
}

static void declaration(compiler *c)
{

	if (match(TOKEN_INCLUDE, &c->parser))
		include_file(c);
	else if (match(TOKEN_FUNC, &c->parser))
		func_declaration(c);
	else if (match(TOKEN_CLASS, &c->parser))
		class_declaration(c);
	else if (match(TOKEN_VAR, &c->parser))
		var_dec(c);
	else
		statement(c);

	if (c->parser.flag)
		synchronize(&c->parser);
}

static void class_declaration(compiler *c)
{
	consume(TOKEN_ID, "ERROR: Expect class name.", &c->parser);

	_key   ar          = parse_id(c);
	class *classc      = _class(ar);
	classc->closures   = GROW_TABLE(NULL, INIT_SIZE);
	class_compiler *cc = ALLOC(sizeof(class_compiler));

	if (find_entry(&c->lookup, &ar).type != T_NULL)
		exit_error("Duplicate class declaration: %s\n", ar.val);

	write_table(c->base->lookup, ar, NumType(c->base->count.obj, T_CLASS));

	c->base->stack.class[c->base->count.obj] = NULL;
	c->base->stack.class[c->base->count.obj] = classc;

	cc->name          = ar;
	cc->index         = c->base->count.obj;
	cc->enclosing     = c->class_compiler;
	c->class_compiler = cc;

	emit_bytes(c, OP_SET_OBJ, c->base->count.obj++);
	emit_byte(c, add_constant(&c->func->ch, GEN(classc, T_CLASS)));

	if (match(TOKEN_CH_SEMI, &c->parser))
	{
		c->class_compiler = c->class_compiler->enclosing;
		return;
	}

	consume(TOKEN_CH_LCURL, "ERROR: Expect ze `{` curl brace", &c->parser);

	while (!check(TOKEN_CH_RCURL, &c->parser) && !check(TOKEN_EOF, &c->parser)
	)
		method(c, classc);

	consume(TOKEN_CH_RCURL, "ERROR: Expect ze `}` curl brace", &c->parser);
	c->class_compiler = c->class_compiler->enclosing;
}

static void method(compiler *c, class *class)
{
	consume(TOKEN_ID, "ERROR: Expect method identifier.", &c->parser);

	_key       ar   = parse_id(c);
	compiler_t type = COMPILER_TYPE_INIT;

	if (ar.hash != c->base->hash.init)
		type = COMPILER_TYPE_METHOD;

	method_body(c, type, ar, &class);
}

static void method_body(compiler *c, compiler_t type, _key ar, class **class)
{
	compiler co;
	init_compiler(&co, c, type, ar);

	c = &co;
	begin_scope(c);
	consume(TOKEN_CH_LPAREN, "Expect `(` after function name.", &c->parser);
	if (!check(TOKEN_CH_RPAREN, &c->parser))
		do
		{
			c->func->arity++;
			if (c->func->arity > 255)
				current_err(
				    "Cannot declare more than 255 function "
				    "parameters",
				    &c->parser
				);
			func_var(c);

		} while (match(TOKEN_CH_COMMA, &c->parser));
	consume(
	    TOKEN_CH_RPAREN, "Expect `)` after function parameters.", &c->parser
	);
	consume(TOKEN_CH_LCURL, "Expect `{` prior to function body.", &c->parser);

	parse_block(c);

	compiler *tmp = c;
	function *f   = end_compile(c);

	closure *clos = _closure(f);

	write_table((*class)->closures, ar, GEN(clos, T_CLOSURE));

	end_scope(c);

	if (type == COMPILER_TYPE_INIT)
		(*class)->init = clos;

	c = c->enclosing;

	emit_bytes(
	    c, OP_METHOD, add_constant(&c->func->ch, GEN(clos, T_CLOSURE))
	);

	for (int i = 0; i < tmp->count.upvalue; i++)
		emit_bytes(
		    c, tmp->stack.upvalue[i].islocal ? 1 : 0,
		    (uint8_t)tmp->stack.upvalue[i].index
		);
}

static void call(compiler *c)
{
	uint8_t argc = argument_list(c);
	emit_bytes(c, OP_CALL, argc);
}
static void call_instance(compiler *c)
{
	uint8_t argc = argument_list(c);
	emit_bytes(c, OP_INSTANCE, argc);
}

static int argument_list(compiler *c)
{
	uint8_t argc = 0;

	if (match(TOKEN_CH_RPAREN, &c->parser))
		return 0;
	do
	{
		expression(c);
		if (argc == 255)
			current_err(
			    "Cannot pass more than 255 function parameters",
			    &c->parser
			);
		argc++;

	} while (match(TOKEN_CH_COMMA, &c->parser));
	consume(TOKEN_CH_RPAREN, "Expect `)` after function args", &c->parser);
	return argc;
}

static void func_declaration(compiler *c)
{
	consume(TOKEN_ID, "Expect function name.", &c->parser);
	_key ar = parse_id(c);

	if (find_entry(&c->base->lookup, &ar).type != T_NULL)
		exit_error("Duplicate function declarations");

	write_table(
	    c->base->lookup, ar, NumType(c->base->count.obj++, T_FUNCTION)
	);
	func_body(c, ar);
}

static void func_body(compiler *c, _key ar)
{
	compiler co;
	init_compiler(&co, c, COMPILER_TYPE_FUNCTION, ar);

	int obj_count = c->base->count.obj - 1;

	c = &co;
	begin_scope(c);
	consume(TOKEN_CH_LPAREN, "Expect `(` after function name.", &c->parser);
	if (!check(TOKEN_CH_RPAREN, &c->parser))
		do
		{
			c->func->arity++;
			if (c->func->arity > 255)
				current_err(
				    "Cannot declare more than 255 function "
				    "parameters",
				    &c->parser
				);
			func_var(c);

		} while (match(TOKEN_CH_COMMA, &c->parser));
	consume(
	    TOKEN_CH_RPAREN, "Expect `)` after function parameters.", &c->parser
	);
	consume(TOKEN_CH_LCURL, "Expect `{` prior to function body.", &c->parser);

	parse_block(c);

	compiler *tmp = c;
	function *f   = end_compile(c);

	closure *clos = _closure(f);
	end_scope(c);

	c = c->enclosing;

	int arg = add_constant(&c->func->ch, GEN(clos, T_CLOSURE));

	emit_bytes(c, OP_CLOSURE, arg);

	emit_byte(c, obj_count);

	for (int i = 0; i < tmp->count.upvalue; i++)
		emit_bytes(
		    c, tmp->stack.upvalue[i].islocal ? 1 : 0,
		    tmp->stack.upvalue[i].index
		);
}

static void func_var(compiler *c)
{
	consume(TOKEN_ID, "Expect variable name.", &c->parser);

	_key ar = parse_id(c);

	int glob = parse_var(c, ar);

	uint8_t set = OP_SET_FUNC_VAR;

	if (glob == -1)
	{
		glob = resolve_local(c, &ar);
		set  = OP_SET_LOCAL_PARAM;
	}
	emit_bytes(c, set, (uint8_t)glob);
}

static void var_dec(compiler *c)
{
	consume(TOKEN_ID, "Expect variable name.", &c->parser);

	_key ar   = parse_id(c);
	int  glob = parse_var(c, ar);

	uint8_t set = 0;
	if (glob != -1)
		set = OP_GLOBAL_DEF;
	else
	{
		glob = resolve_local(c, &ar);
		set  = OP_SET_LOCAL;
	}

	if (match(TOKEN_OP_ASSIGN, &c->parser))
	{
		expression(c);
		emit_bytes(c, set, (uint8_t)glob);
	}
	else
		emit_byte(c, OP_NOOP);

	consume(
	    TOKEN_CH_SEMI, "Expect ';' after variable declaration.", &c->parser
	);
}

static void synchronize(parser *parser)
{
	parser->flag = 0;

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
		default:
			break;
		}
		advance_compiler(parser);
	}
}

static void statement(compiler *c)
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
		advance_compiler(&c->parser);
	else
		default_expression(c);
}

static void rm_statement(compiler *c)
{
	consume(
	    TOKEN_CH_LPAREN, "Expect `(` prior to rm expression.", &c->parser
	);
	if (match(TOKEN_ID, &c->parser))
		;

	_key    ar = parse_id(c);
	uint8_t get;
	int     arg = resolve_local(c, &ar);

	if (arg != -1)
		get = OP_GET_LOCAL;
	else if ((arg = resolve_upvalue(c, &ar)) != -1)
		get = OP_GET_UPVALUE;
	else
	{
		arg = add_constant(&c->func->ch, KEY(ar));
		get = OP_GET_GLOBAL;
	}
	emit_bytes(c, get, (uint8_t)arg);
	emit_byte(c, OP_RM);
	consume(TOKEN_CH_RPAREN, "Expect `)` after rm statement", &c->parser);
	consume(TOKEN_CH_SEMI, "Expect `;` at end of statement", &c->parser);
}

static inline bool is_comment(parser *parser)
{
	return check(TOKEN_LINE_COMMENT, parser) ||
	       check(TOKEN_NLINE_COMMENT, parser);
}

static void for_statement(compiler *c)
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

	int start = c->func->ch.count;
	int exit  = -1;
	if (!match(TOKEN_CH_SEMI, &c->parser))
	{
		expression(c);
		consume(
		    TOKEN_CH_SEMI, "Expect `;` after 'for' condition.", &c->parser
		);
		exit = emit_jump(c, OP_JMPF);
	}
	if (!match(TOKEN_CH_RPAREN, &c->parser))
	{
		int body_jump = emit_jump(c, OP_JMP);
		int inc_start = c->func->ch.count;

		expression(c);
		emit_byte(c, OP_POP);
		consume(
		    TOKEN_CH_RPAREN, "Expect `)` after 'for' statement.", &c->parser
		);

		emit_loop(c, start);
		start = inc_start;
		patch_jump(c, body_jump);
	}

	statement(c);
	emit_loop(c, start);

	if (exit != -1)
		patch_jump(c, exit);

	end_scope(c);
}

static void while_statement(compiler *c)
{
	int start = c->func->ch.count;

	consume(TOKEN_CH_LPAREN, "Expect `(` after 'while'.", &c->parser);
	expression(c);
	consume(
	    TOKEN_CH_RPAREN, "Expect `)` after 'while' condition.", &c->parser
	);

	int exit_jmp = emit_jump(c, OP_JMPF);

	statement(c);

	// If falsey, loop
	emit_loop(c, start);

	// If true, exit
	patch_jump(c, exit_jmp);
}

static void consume_if(compiler *c)
{
	consume(TOKEN_CH_LPAREN, "Expect `(` after an 'if'.", &c->parser);
	expression(c);
	consume(
	    TOKEN_CH_RPAREN, "Expect `)` after an 'if' condtion.", &c->parser
	);
}
static void consume_elif(compiler *c)
{
	consume(TOKEN_CH_LPAREN, "Expect `(` after an 'elif'.", &c->parser);
	expression(c);
	consume(
	    TOKEN_CH_RPAREN, "Expect `)` after an 'elif' condtion.", &c->parser
	);
}

static void consume_switch(compiler *c)
{
	consume(TOKEN_CH_LPAREN, "Expect `(` after a 'switch'.", &c->parser);
	id(c);
	consume(
	    TOKEN_CH_RPAREN, "Expect `)` after a 'switch' condtion.", &c->parser
	);
}

static void switch_statement(compiler *c)
{
	consume_switch(c);
	match(TOKEN_CH_LCURL, &c->parser);

	case_statement(c);

	if (match(TOKEN_DEFAULT, &c->parser))
	{
		consume(
		    TOKEN_CH_COLON, "Expect `:` prior to case body.", &c->parser
		);
		while (!match(TOKEN_CH_RCURL, &c->parser))
			statement(c);
	}
	match(TOKEN_CH_RCURL, &c->parser);

	c->func->ch.cases.bytes[c->func->ch.cases.count++] = c->func->ch.count;
}

static void case_statement(compiler *c)
{
	uint8_t get = c->array.get;
	uint8_t arg = c->array.index;

	while (match(TOKEN_CASE, &c->parser))
	{
		expression(c);
		consume(
		    TOKEN_CH_COLON, "Expect `:` prior to case body.", &c->parser
		);
		emit_bytes(c, get, (uint8_t)arg);
		emit_byte(c, OP_EQ);

		int exit = emit_jump(c, OP_JMPF);

		while (!check(TOKEN_BREAK, &c->parser))
		{
			statement(c);

			if (match(TOKEN_CH_RCURL, &c->parser))
				break;
			if (match(TOKEN_BREAK, &c->parser))
			{
				consume(
				    TOKEN_CH_SEMI,
				    "Expected a semi colon after break "
				    "statement",
				    &c->parser
				);
				break;
			}
		}
		emit_byte(c, OP_JMPL);
		emit_bytes(
		    c, (c->func->ch.cases.count >> 8) & 0xFF,
		    (c->func->ch.cases.count & 0xFF)
		);
		patch_jump(c, exit);
	}
}

static void if_statement(compiler *c)
{

	consume_if(c);

	int fi = emit_jump(c, OP_JMPF);
	statement(c);

	int exit = emit_jump(c, OP_JMP);
	patch_jump(c, fi);

	elif_statement(c);

	if (match(TOKEN_ELSE, &c->parser))
		statement(c);

	c->func->ch.cases.bytes[c->func->ch.cases.count++] = c->func->ch.count;
	patch_jump(c, exit);
}

static void elif_statement(compiler *c)
{
	while (match(TOKEN_ELIF, &c->parser))
	{
		consume_elif(c);
		int exit = emit_jump(c, OP_JMPF);
		statement(c);
		emit_byte(c, OP_JMPL);
		emit_bytes(
		    c, UPPER(c->func->ch.cases.count),
		    LOWER(c->func->ch.cases.count)
		);
		patch_jump(c, exit);
	}
}

static void ternary_statement(compiler *c)
{
	int exit = emit_jump(c, OP_JMPF);
	// emit_byte(c, OP_POP);
	expression(c);
	consume(
	    TOKEN_CH_COLON, "Expect `:` between ternary expressions.", &c->parser
	);
	int tr = emit_jump(c, OP_JMP);
	patch_jump(c, exit);
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
	if (c->meta.type == COMPILER_TYPE_SCRIPT)
		prev_error(
		    "ERROR: Unable to return from top of script.", &c->parser
		);

	else if (match(TOKEN_CH_SEMI, &c->parser))
		emit_return(c);
	else
	{
		if (c->meta.type == COMPILER_TYPE_INIT)
			prev_error(
			    "ERROR: Unable to return value from initializer.",
			    &c->parser
			);
		expression(c);
		consume(
		    TOKEN_CH_SEMI,
		    "ERROR: Expect semi colon after return statement.", &c->parser
		);
		emit_byte(c, OP_RETURN);
	}
}

static void patch_jump(compiler *c, int offset)
{

	int jump = c->func->ch.count - offset - 2;

	if (jump >= INT16_MAX)
		prev_error("ERROR: To great a distance ", &c->parser);

	c->func->ch.ip[offset]     = UPPER(jump);
	c->func->ch.ip[offset + 1] = LOWER(jump);
}

static void emit_loop(compiler *c, int byte)
{
	emit_byte(c, OP_LOOP);
	int offset = c->func->ch.count - byte + 2;

	if (offset > UINT16_MAX)
		prev_error("ERROR: big boi loop", &c->parser);

	emit_bytes(c, (offset >> 8) & 0xFF, offset & 0xFF);
}

static int emit_jump(compiler *c, int byte)
{
	emit_byte(c, byte);
	emit_bytes(c, 0xFF, 0xFF);

	return c->func->ch.count - 2;
}

static void default_expression(compiler *c)
{
	expression(c);
	consume(TOKEN_CH_SEMI, "Expect `;` after expression.", &c->parser);
}

static void print_statement(compiler *c)
{
	consume(
	    TOKEN_CH_LPAREN, "Expect `(` prior to print expression", &c->parser
	);
	do
	{
		expression(c);

		if (check(TOKEN_CH_COMMA, &c->parser))
			emit_byte(c, OP_PRINT);
	} while (match(TOKEN_CH_COMMA, &c->parser));

	emit_byte(c, OP_PRINT);

	consume(TOKEN_CH_RPAREN, "Expect `)` after print expression", &c->parser);
	consume(TOKEN_CH_SEMI, "Expect ';' after value.", &c->parser);
}

static void begin_scope(compiler *c)
{
	c->count.scope++;
}
static void end_scope(compiler *c)
{

	c->count.scope--;
	if (c->count.local > 0 &&
	    (c->stack.local[c->count.local - 1].depth > c->count.scope))
		emit_bytes(
		    c, OP_POPN, add_constant(&c->func->ch, Num(c->count.local))
		);
}

static void parse_block(compiler *c)
{
	while (!check(TOKEN_CH_RCURL, &c->parser) && !check(TOKEN_EOF, &c->parser)
	)
		declaration(c);
	consume(TOKEN_CH_RCURL, "Expect `}` after block statement", &c->parser);
}
static void block(compiler *c)
{
	begin_scope(c);
	parse_block(c);
	end_scope(c);
}

static bool match(token_t t, parser *parser)
{
	if (!check(t, parser))
		return false;
	advance_compiler(parser);
	return true;
}
static bool check(token_t t, parser *parser)
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
	consume(TOKEN_CH_RPAREN, "Expect `)` after expression", &c->parser);
}

static void storage_type(compiler *c)
{
	switch (c->parser.cur.type)
	{
	case TOKEN_STORAGE_TYPE_NUM:
	case TOKEN_STORAGE_TYPE_CHAR:
	case TOKEN_STORAGE_TYPE_BOOL:
	case TOKEN_STORAGE_TYPE_STR:
		advance_compiler(&c->parser);
		return;
	default:
		error("Expected a destination storage type");
		exit(1);
	}
}

static void cast(compiler *c)
{
	token_t from = c->parser.pre.type;

	consume(
	    TOKEN_OP_CAST,
	    "Expected `->` cast operator prior to destination type", &c->parser
	);

	storage_type(c);

	token_t to = c->parser.pre.type;

	expression(c);

	cast_t op;
	switch (from)
	{
	case TOKEN_STORAGE_TYPE_NUM:
		switch (to)
		{
		case TOKEN_STORAGE_TYPE_CHAR:
			op = CAST_NUM_CHAR;
			break;
		case TOKEN_STORAGE_TYPE_STR:
			op = CAST_NUM_STR;
			break;
		default:
			return;
		}
		break;
	case TOKEN_STORAGE_TYPE_CHAR:
		switch (to)
		{
		case TOKEN_STORAGE_TYPE_NUM:
			op = CAST_CHAR_NUM;
			break;
		case TOKEN_STORAGE_TYPE_STR:
			op = CAST_CHAR_STR;
			break;
		default:
			return;
		}
		break;
	case TOKEN_STORAGE_TYPE_BOOL:
		switch (to)
		{
		case TOKEN_STORAGE_TYPE_NUM:
			op = CAST_BOOL_NUM;
			break;
		case TOKEN_STORAGE_TYPE_STR:
			op = CAST_BOOL_STR;
			break;
		default:
			return;
		}
		break;
	case TOKEN_STORAGE_TYPE_STR:
		switch (to)
		{
		case TOKEN_STORAGE_TYPE_NUM:
			op = CAST_STR_NUM;
			break;
		case TOKEN_STORAGE_TYPE_BOOL:
			op = CAST_STR_BOOL;
			break;
		case TOKEN_STORAGE_TYPE_CHAR:
			op = CAST_STR_CHAR;
			break;
		default:
			return;
		}
		break;

	default:
		return;
	}

	emit_bytes(c, OP_CAST, op);
}

static PRule *get_rule(int t)
{
	return &rules[t];
}

static void parse_precedence(prec_t prec, compiler *c)
{
	advance_compiler(&c->parser);
	parse_fn prefix_rule = get_rule(c->parser.pre.type)->prefix;

	if (!prefix_rule)
	{
		prev_error("ERROR: Expect expression.", &c->parser);
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

static void _and(compiler *c)
{
	int end = emit_jump(c, OP_JMPF);

	parse_precedence(PREC_AND, c);

	patch_jump(c, end);
}
static void _or(compiler *c)
{
	int else_jmp = emit_jump(c, OP_JMPT);

	parse_precedence(PREC_OR, c);

	patch_jump(c, else_jmp);
}

static void binary(compiler *c)
{
	int t = c->parser.pre.type;

	PRule *rule = get_rule(t);
	parse_precedence((prec_t)rule->prec + 1, c);

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

	default:
		return;
	}
}
static void unary(compiler *c)
{
	int op = c->parser.pre.type;

	parse_precedence(PREC_UNARY, c);

	switch (op)
	{

	case TOKEN_OP_INC:
		emit_byte(c, OP_INC);
		emit_bytes(c, c->array.set, c->array.index);
		break;
	case TOKEN_OP_DEC:
		emit_byte(c, OP_DEC);
		emit_bytes(c, c->array.set, c->array.index);
		break;
	case TOKEN_OP_SUB:
	case TOKEN_OP_BANG:
		emit_byte(c, OP_NEG);
		break;
	default:
		return;
	}
}

static void compound_assign(compiler *c)
{
	int op = c->parser.pre.type;

	parse_precedence(PREC_ASSIGNMENT, c);

	switch (op)
	{
	case TOKEN_ADD_ASSIGN:
		emit_byte(c, OP_ADD);
		break;
	case TOKEN_SUB_ASSIGN:
		emit_byte(c, OP_SUB);
		break;
	case TOKEN_MUL_ASSIGN:
		emit_byte(c, OP_MUL);
		break;
	case TOKEN_DIV_ASSIGN:
		emit_byte(c, OP_DIV);
		break;
	case TOKEN_MOD_ASSIGN:
		emit_byte(c, OP_MOD);
		break;
	case TOKEN_AND_ASSIGN:
		emit_byte(c, OP_AND);
		break;
	case TOKEN_OR__ASSIGN:
		emit_byte(c, OP_OR);
		break;
	}
	emit_bytes(c, c->array.set, c->array.index);
}

static void infix_unary(compiler *c)
{
	int op = c->parser.pre.type;

	switch (op)
	{
	case TOKEN_OP_INC:
		emit_byte(c, OP_INC);
		break;
	case TOKEN_OP_DEC:
		emit_byte(c, OP_DEC);
		break;
	case TOKEN_OP_SUB:
	case TOKEN_OP_BANG:
		emit_byte(c, OP_NEG);
		break;
	default:
		return;
	}
	emit_bytes(c, c->array.set, c->array.index);
}

static void current_err(const char *err, parser *parser)
{
	error_at(&parser->cur, parser, err);
}
static void prev_error(const char *err, parser *parser)
{
	error_at(&parser->pre, parser, err);
}
static void error_at(Token toke, parser *parser, const char *err)
{
	if (parser->flag)
		return;
	parser->flag = 1;

	fprintf(
	    stderr, "[file: %s, line: %d:%d] Error", parser->current_file,
	    toke->line, toke->col
	);

	if (toke->type == TOKEN_EOF)
		fprintf(stderr, " reached end of file");
	else if (toke->type != TOKEN_ERR)
		fprintf(stderr, " at '%.*s'", toke->size, toke->start);

	fprintf(stderr, ": %s\n", err);
}

static void emit_return(compiler *c)
{
	if (c->meta.type == COMPILER_TYPE_INIT)
		emit_bytes(c, OP_GET_LOCAL, 0);
	emit_byte(c, OP_RETURN);
}
static void emit_byte(compiler *c, uint8_t byte)
{
	write_chunk(&c->func->ch, byte, c->parser.pre.line);
}
static void emit_bytes(compiler *c, uint8_t b1, uint8_t b2)
{
	write_chunk(&c->func->ch, b1, c->parser.pre.line);
	write_chunk(&c->func->ch, b2, c->parser.pre.line);
}
static void emit_constant(compiler *c, element ar)
{
	emit_bytes(c, OP_CONSTANT, add_constant(&c->func->ch, ar));
}

static void num(compiler *c)
{
	emit_constant(c, Num(strtod(c->parser.pre.start, NULL)));
}

static void ch(compiler *c)
{
	emit_constant(c, Char(*++c->parser.pre.start));
}

static void str(compiler *c)
{
	emit_constant(
	    c, String((char *)++c->parser.pre.start, c->parser.pre.size - 2)
	);
}

static void fmt_str(compiler *c)
{

	char *tmp = NULL;
	tmp       = (char *)++c->parser.pre.start;

	uint8_t expr  = 0;
	uint8_t count = 0;

	token t;
	t = c->parser.cur;

	for (; *tmp != '`'; tmp++, count++)
		if (*tmp == '$' && tmp[1] == '{')
		{
			tmp += 2;
			if (count > 0)
			{
				emit_constant(
				    c, String(
					     c->parser.pre.start,
					     (int)(tmp - c->parser.pre.start) - 2
					 )
				);
				if (expr)
					emit_byte(c, OP_ADD);
			}
			c->parser.pre.start = tmp;

			for (; *tmp != '}'; tmp++)
				;
			element str = String(
			    c->parser.pre.start, (int)(tmp - c->parser.pre.start)
			);

			re_init_scanner(str.val.String, t.line);
			c->parser.cur = scan_token();
			expression(c);
			emit_byte(c, OP_TO_STR);
			FREE_OBJ(str);

			if (count > 0)
			{
				emit_byte(c, OP_ADD);
				count = 0;
			}
			c->parser.pre.start = tmp + 1;

			expr = 1;
			if (*++tmp == '`')
				break;
		}
	if (count > 0)
	{
		emit_constant(
		    c, String(c->parser.pre.start, (int)(tmp - c->parser.pre.start))
		);
		if (expr)
			emit_byte(c, OP_ADD);
	}

	re_init_scanner(t.start, t.line);
	c->parser.cur = scan_token();
}

static void boolean(compiler *c)
{
	if (*c->parser.pre.start == 'n')
		emit_bytes(c, OP_CONSTANT, add_constant(&c->func->ch, Null()));
	else
		emit_constant(c, Bool(*c->parser.pre.start == 't' ? true : false));
}

static _key parse_string(compiler *c)
{
	return Key((char *)++c->parser.pre.start, c->parser.pre.size - 2);
}

static void _table(compiler *c)
{
	table *t = NULL;
	consume(
	    TOKEN_CH_LPAREN, "Expect `(` prior to table allocation", &c->parser
	);

	if (match(TOKEN_CH_RPAREN, &c->parser))
	{
		t = GROW_TABLE(NULL, STACK_SIZE);
		emit_bytes(
		    c, OP_CONSTANT, add_constant(&c->func->ch, GEN(t, T_TABLE))
		);
		return;
	}
	else
	{
		expression(c);
		emit_byte(c, OP_ALLOC_TABLE);
		consume(
		    TOKEN_CH_RPAREN, "Expect `)` after table declaration",
		    &c->parser
		);
	}
}

static int resolve_native(compiler *c, _key *ar)
{

	element el = find_entry(&c->base->lookup, ar);

	if (el.type == T_NATIVE)
		return el.val.Num;

	return -1;
}
static void parse_native_var_arg(compiler *c)
{

	_key key = parse_id(c);
	int  arg = resolve_native(c, &key);
	emit_bytes(c, OP_GET_OBJ, (uint8_t)arg);
	consume(TOKEN_CH_LPAREN, "Expect `(` prior to function call", &c->parser);
	call(c);
}

static _key parse_id(compiler *c)
{
	return Key(c->parser.pre.start, c->parser.pre.size);
}

static int resolve_call(compiler *c, _key *ar)
{

	element el = find_entry(&c->base->lookup, ar);

	if (el.type == T_FUNCTION)
		return el.val.Num;

	return -1;
}

static void _array(compiler *c)
{
	int i = 1;

	if (match(TOKEN_CH_RSQUARE, &c->parser))
	{
		emit_byte(c, OP_ALLOC_VECTOR);
		return;
	}
	do
	{
		expression(c);
		i++;
	} while (match(TOKEN_CH_COMMA, &c->parser));

	consume(
	    TOKEN_CH_RSQUARE, "Expect closing brace after array declaration",
	    &c->parser
	);

	emit_byte(c, OP_INIT_VECTOR);
	emit_bytes(c, UPPER(i), LOWER(i));
}
static void _access(compiler *c)
{

	expression(c);

	consume(
	    TOKEN_CH_RSQUARE, "Expect closing brace prior to array access",
	    &c->parser
	);

	if (match(TOKEN_OP_ASSIGN, &c->parser))
	{
		expression(c);
		emit_byte(c, OP_SET_ACCESS);
	}
	else
		emit_byte(c, OP_GET_ACCESS);
}

static int resolve_class(compiler *c, _key *ar)
{
	element el = find_entry(&c->base->lookup, ar);

	if (el.type == T_CLASS)
		return el.val.Num;

	return -1;
}

static void dot(compiler *c)
{
	match(TOKEN_ID, &c->parser);

	_key ar = parse_id(c);

	int arg = add_constant(&c->func->ch, KEY(ar));

	c->array.get   = OP_GET_PROP;
	c->array.set   = OP_SET_PROP;
	c->array.index = arg;

	if (match(TOKEN_OP_ASSIGN, &c->parser))
	{
		expression(c);
		emit_bytes(c, OP_SET_PROP, arg);
	}
	else
		emit_bytes(c, OP_GET_PROP, arg);
}

static void _this(compiler *c)
{
	if (!c->class_compiler)
	{
		prev_error(
		    "ERROR: can't use `this` keyword outside of a class "
		    "body.",
		    &c->parser
		);
		return;
	}
	emit_byte(c, OP_THIS);
}

static void id(compiler *c)
{

	match(TOKEN_ID, &c->parser);

	_key    ar  = parse_id(c);
	uint8_t get = OP_GET_GLOBAL, set = OP_SET_GLOBAL;

	int arg = resolve_call(c, &ar);

	if (arg != -1)
	{
		emit_bytes(c, OP_GET_OBJ, (uint8_t)arg);
		return;
	}

	if ((arg = resolve_class(c, &ar)) != -1)
	{
		uint8_t init = 0;

		if (c->base->stack.class[arg] -> init)
		{
			match(TOKEN_CH_LPAREN, &c->parser);
			emit_bytes(c, OP_CLASS, (uint8_t)arg);
			call(c);
			init = 1;
		}

		emit_bytes(c, OP_ALLOC_INSTANCE, (uint8_t)arg);
		emit_byte(c, init);

		if (match(TOKEN_CH_LPAREN, &c->parser))
			call_instance(c);
		return;
	}

	if ((arg = resolve_local(c, &ar)) != -1)
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
		arg = add_constant(&c->func->ch, KEY(ar));

	c->array.set   = set;
	c->array.get   = get;
	c->array.index = arg;

	if (match(TOKEN_OP_ASSIGN, &c->parser))
	{
		expression(c);
		emit_bytes(c, set, (uint8_t)arg);
	}
	else
		emit_bytes(c, get, (uint8_t)arg);
}

static int parse_var(compiler *c, _key ar)
{
	declare_var(c, ar);
	if (c->count.scope > 0)
		return -1;
	return add_constant(&c->func->ch, KEY(ar));
}

static bool idcmp(_key a, _key b)
{
	return a.hash == b.hash;
}

static int resolve_local(compiler *c, _key *name)
{
	for (int i = c->count.local - 1; i >= 0; i--)
		if (idcmp(*name, c->stack.local[i].name))
			return i;
	return -1;
}
static int resolve_upvalue(compiler *c, _key *name)
{

	if (!c->enclosing)
		return -1;

	int local = resolve_local(c->enclosing, name);

	if (local != -1)
	{
		c->enclosing->stack.local[local].captured = true;
		return add_upvalue(c, (uint8_t)local, true);
	}

	int upvalue = resolve_upvalue(c->enclosing, name);
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
		prev_error(
		    "ERROR: To many closure variables in function.", &c->parser
		);
		return 0;
	}

	c->stack.upvalue[c->count.upvalue].islocal = islocal;
	c->stack.upvalue[c->count.upvalue].index   = (uint8_t)index;
	c->count.upvalue++;
	return c->count.upvalue++;
}

static void declare_var(compiler *c, _key ar)
{
	if (c->count.scope == 0)
		return;

	for (int i = c->count.local - 1; i >= 0; i--)
	{
		local *_local = &c->stack.local[i];

		if (_local->depth != 0 && _local->depth < c->count.scope)
			break;

		if (idcmp(ar, _local->name))
			prev_error(
			    "ERROR: Duplicate variable identifiers in scope",
			    &c->parser
			);
	}

	add_local(c, &ar);
}

static void add_local(compiler *c, _key *ar)
{
	if (c->count.local == LOCAL_COUNT)
	{
		prev_error(
		    "ERROR: Too many local variables in function.", &c->parser
		);
		return;
	}
	c->stack.local[c->count.local].name     = *ar;
	c->stack.local[c->count.local].depth    = c->count.scope;
	c->stack.local[c->count.local].captured = false;
	c->count.local++;
}
static function *end_compile(compiler *a)
{
	function *f = a->func;

	emit_return(a);
#ifdef DEBUG_PRINT_CODE
	if (!a->parser.flag)
		disassemble_chunk(&a->func->ch, a->func->name.as.String);
#endif
	if (a->enclosing)
	{

		parser tmp = a->parser;
		a          = a->enclosing;
		a->parser  = tmp;
	}

	return f;
}

function *compile(const char *src, table **lookup)
{
	compiler c;

	init_scanner(src);
	init_compiler(&c, NULL, COMPILER_TYPE_SCRIPT, Key("SCRIPT", 6));

	c.base            = &c;
	c.base->lookup    = NULL;
	c.base->lookup    = *lookup;
	c.base->count.obj = 3;
	c.hash.init       = hash_key("init");
	c.hash.len        = hash_key("len");
	c.hash.push       = hash_key("push");
	c.hash.pop        = hash_key("pop");

	c.parser.flag         = false;
	c.parser.current_file = NULL;

	// write_table(
	//     c.base->lookup, Key("clock", 5),
	//     NumType(c.base->count.obj++, T_NATIVE)
	// );
	// write_table(
	//     c.base->lookup, Key("square", 6),
	//     NumType(c.base->count.obj++, T_NATIVE)
	// );
	// write_table(
	//     c.base->lookup, Key("file", 4), NumType(c.base->count.obj++,
	//     T_NATIVE)
	// );

	advance_compiler(&c.parser);

	while (!match(TOKEN_EOF, &c.parser))
		declaration(&c);
	consume(TOKEN_EOF, "Expect end of expression", &c.parser);

	function *f = end_compile(&c);

	FREE((char *)(c.parser.current_file));
	FREE(c.base->lookup->records);
	FREE(c.base->lookup);

	return c.parser.flag ? NULL : f;
}

function *compile_path(const char *src, const char *path, const char *name)
{
	compiler c;

	init_scanner(src);

	init_compiler(&c, NULL, COMPILER_TYPE_SCRIPT, Key("SCRIPT", 6));

	c.meta.cwd = path;

	c.base         = &c;
	c.base->lookup = NULL;
	c.base->lookup = GROW_TABLE(NULL, INIT_SIZE);

	c.hash.init = hash_key("init");
	c.hash.len  = hash_key("len");
	c.hash.push = hash_key("push");
	c.hash.pop  = hash_key("pop");

	c.parser.flag         = false;
	c.parser.current_file = name;

	write_table(
	    c.base->lookup, Key("clock", 5),
	    NumType(c.base->count.obj++, T_NATIVE)
	);
	write_table(
	    c.base->lookup, Key("square", 6),
	    NumType(c.base->count.obj++, T_NATIVE)
	);
	write_table(
	    c.base->lookup, Key("file", 4), NumType(c.base->count.obj++, T_NATIVE)
	);

	advance_compiler(&c.parser);

	while (!match(TOKEN_EOF, &c.parser))
		declaration(&c);
	consume(TOKEN_EOF, "Expect end of expression", &c.parser);

	function *f = end_compile(&c);
	f->objc     = c.base->count.obj;

	FREE((char *)(c.parser.current_file));
	FREE(c.base->lookup->records);
	FREE(c.base->lookup);

	return c.parser.flag ? NULL : f;
}
