#include "mem.h"
#include "parser.h"
#include "parser_util.h"

static void init_parser(parser *parser, const char *file)
{
	advance(parser);
	parser->file = file;
	parser->ast  = NULL;
	parser->_obj = 0;

	parser->lookup = NULL;
	parser->lookup = alloc_table(INIT_SIZE);
	parser->ast    = parser_ast_stack(STACK_SIZE);
}
static body parse_id(token toke, ast_t type)
{
	return _ast(_ast_node(toke), type);
}

static body class_declaration(parser *parser)
{
}
static body func_declaration(parser *parser)
{
	token      t    = parser->cur;
	ast_stack *s    = NULL;
	uint16_t   line = parser->cur.line;

	s = init_ast_stack(_MIN_BLOCK_SIZE);

	key k = _key((char *)t.start, t.size);

	element el;
	if ((el = find_entry(&parser->lookup, k.key)).type == T_NULL)
	{
		el      = OBJ(_number(parser->_obj++));
		el.type = T_CLOSURE;
		write_table(&parser->lookup, k, el);
	}

	push_ast(&s, parse_id(t, AST_FUNC_ID));
	advance(parser);

	consume(
	    TOKEN_CH_LPAREN, "Expected an open brace `(` prior to function args.",
	    parser
	);
	if (!match(TOKEN_CH_RPAREN, parser))
	{
		do
		{
			push_ast(&s, parse_id(parser->cur, AST_PARAM));
		} while (match(TOKEN_CH_COMMA, parser));

		consume(
		    TOKEN_CH_RPAREN,
		    "Expected a closing brace `)` after function args.", parser
		);
	}

	if (match(TOKEN_CH_SEMI, parser))
		return _ast_block(s, AST_FUNC_FD, line);

	consume(
	    TOKEN_CHAR_TYPE_LCURL, "Expect an `{` prior to function body", parser
	);
	push_ast(&s, _block(parser));

	return _ast_block(s, AST_FUNC, line);
}

static uint8_t assignment_op(parser *p)
{
	switch (p->cur.type)
	{
	case TOKEN_OP_ASSIGN:
	case TOKEN_ADD_ASSIGN:
	case TOKEN_SUB_ASSIGN:
	case TOKEN_MUL_ASSIGN:
	case TOKEN_DIV_ASSIGN:
	case TOKEN_MOD_ASSIGN:
	case TOKEN_AND_ASSIGN:
	case TOKEN_OR__ASSIGN:
		advance(p);
		return 1;
	default:
		return 0;
	}
}

static body variable_declaration(parser *parser)
{

	ast *s = NULL;

	s = expression(parser);
	consume(TOKEN_CH_SEMI, "Missing semicolon", parser);
	return _ast(s, AST_DECLARATION);
}

static body declaration(parser *parser)
{

	body b;

	if (match(TOKEN_FUNC, parser))
		b = func_declaration(parser);
	else if (match(TOKEN_VAR, parser))
		b = variable_declaration(parser);
	else if (match(TOKEN_CLASS parser))
		b = class_declaration(parser);
	else
		b = statement(parser);

	// push_ast(&parser->ast, b);
	return b;
}

static body generic_expression(parser *parser, ast_t type)
{
	ast *a = NULL;
	consume(
	    TOKEN_CH_LPAREN, "Expect a `(` prior to statement condition", parser
	);
	a = expression(parser);
	consume(
	    TOKEN_CH_RPAREN, "Expect a `)` following a statement condition",
	    parser
	);
	return _ast(a, type);
}
static body _block(parser *parser)
{
	ast_stack *s  = NULL;
	s             = init_ast_stack(_MIN_BLOCK_SIZE);
	uint16_t line = parser->cur.line;

	while (!match(TOKEN_CHAR_TYPE_RCURL, parser))
	{
		body tmp = declaration(parser);
		push_ast(&s, tmp);
	}

	return _ast_block(s, AST_BLOCK, line);
}

static body _print(parser *parser)
{
	body b = generic_expression(parser, AST_PRINT);
	consume(TOKEN_CH_SEMI, "Expect `;` after expression.", parser);
	return b;
}

static void _if_body(parser *parser, ast_stack **stk)
{
	if (match(TOKEN_CHAR_TYPE_LCURL, parser))
		push_ast(stk, _block(parser));
	else
		push_ast(stk, statement(parser));
}
static body _if(parser *parser)
{
	ast_stack *stk  = NULL;
	uint16_t   line = parser->cur.line;
	stk             = init_ast_stack(_MIN_BLOCK_SIZE);
	push_ast(&stk, generic_expression(parser, AST_CONDITION));

	_if_body(parser, &stk);
	while (match(TOKEN_TYPE_ELSE_IF, parser))
	{
		push_ast(&stk, generic_expression(parser, AST_CONDITION));
		_if_body(parser, &stk);
	}
	if (match(TOKEN_TYPE_ELSE, parser))
		_if_body(parser, &stk);

	return _ast_block(stk, AST_IF, line);
}
static body _switch(parser *parser)
{
	ast_stack *stk  = NULL;
	uint16_t   line = parser->cur.line;
	stk             = init_ast_stack(_MIN_BLOCK_SIZE);

	push_ast(&stk, generic_expression(parser, AST_SWITCH));

	consume(
	    TOKEN_CHAR_TYPE_LCURL,
	    "Expected an `{` open brace. Invalid switch statement syntax", parser
	);
	while (match(TOKEN_TYPE_CASE, parser))
	{

		push_ast(&stk, _ast(expression(parser), AST_CASE));
		consume(
		    TOKEN_CHAR_TYPE_COLON, "Expected a `:` after case expression",
		    parser
		);

		while (!match(TOKEN_TYPE_BREAK, parser))
			push_ast(&stk, statement(parser));
	}
	if (match(TOKEN_TYPE_DEFAULT, parser))
	{
		consume(
		    TOKEN_CHAR_TYPE_COLON, "Expected a `:` after default Longword",
		    parser
		);
		while (!match(TOKEN_CHAR_TYPE_RCURL, parser))
			push_ast(&stk, statement(parser));

		return _ast_block(stk, AST_SWITCH, line);
	}

	consume(
	    TOKEN_CHAR_TYPE_RCURL,
	    "Expected a `}` closing brace. Invalid switch statement syntax",
	    parser
	);

	return _ast_block(stk, AST_SWITCH, line);
}
static body _while(parser *parser)
{
	ast_stack *stk  = NULL;
	uint16_t   line = parser->cur.line;
	stk             = init_ast_stack(_MIN_BLOCK_SIZE);
	push_ast(&stk, generic_expression(parser, AST_WHILE));

	_if_body(parser, &stk);

	return _ast_block(stk, AST_WHILE, line);
}

static body _for(parser *parser)
{
	ast_stack *stk  = NULL;
	uint16_t   line = parser->cur.line;
	stk             = init_ast_stack(_MIN_BLOCK_SIZE);

	consume(
	    TOKEN_CH_LPAREN, "Expected a `(`. Invalid for statement syntax",
	    parser
	);
	if (!match(TOKEN_CH_SEMI, parser))
	{
		if (match(TOKEN_TYPE_LET, parser))
			push_ast(&stk, variable_declaration(parser));
		else
			push_ast(&stk, default_expression(parser));
	}
	if (!match(TOKEN_CH_SEMI, parser))
		push_ast(&stk, default_expression(parser));

	while (match(TOKEN_CH_COMMA, parser))
		push_ast(&stk, default_expression(parser));

	consume(
	    TOKEN_CH_RPAREN, "Expected a `)`. Invalid for statement syntax",
	    parser
	);

	_if_body(parser, &stk);

	return _ast_block(stk, AST_FOR, line);
}

static body default_expression(parser *parser)
{
	body b = _ast(expression(parser), AST_DEF);
	consume(TOKEN_CH_SEMI, "Expect `;` after expression.", parser);

	return b;
}
static body _return(parser *parser)
{
}

static body statement(parser *parser)
{

	body b;
	if (match(TOKEN_TYPE_PRINT, parser))
		b = _print(parser);
	else if (match(TOKEN_TYPE_IF, parser))
		b = _if(parser);
	else if (match(TOKEN_TYPE_SWITCH, parser))
		b = _switch(parser);
	else if (match(TOKEN_TYPE_WHILE, parser))
		b = _while(parser);
	else if (match(TOKEN_TYPE_FOR, parser))
		b = _for(parser);
	else if (match(TOKEN_TYPE_RETURN, parser))
		b = _return(parser);

	else
		b = default_expression(parser);
	return b;
}

static void error_at(token *toke, parser *parser, const char *err)
{

	fprintf(
	    stderr, "[file: %s, line: %d] Error", parser->file, toke->line - 1
	);

	if (toke->type == TOKEN_TYPE_EOF)
		fprintf(stderr, " at end");
	else if (toke->type != TOKEN_TYPE_ERROR)
		fprintf(stderr, " at '%.*s'", (int)toke->size, toke->start);

	fprintf(stderr, ": %s\n", err);
}
static void current_err(const char *err, parser *parser)
{
	error_at(&parser->cur, parser, err);
}
static void err(const char *err, parser *parser)
{
	error_at(&parser->prev, parser, err);
}

static PRule *_rule(token token)
{
	return &rules[token.type];
}
static prec_t _prec(token token)
{
	return rules[token.type].prec;
}
static parse_fn _infix(token token)
{
	return _rule(token)->infix;
}
static parse_fn _prefix(token token)
{
	return _rule(token)->prefix;
}

static void advance(parser *parser)
{
	parser->prev = parser->cur;
	parser->cur  = scan_token();

	if (parser->cur.type == TOKEN_TYPE_EOF)
		return;
	if (parser->cur.type == TOKEN_TYPE_ERROR)
		exit_error("Parser encountered invalid token\n");
	if (parser->cur.type == TOKEN_TYPE_NULL)
		advance(parser);
}

static ast *expression(parser *parser)
{
	return parse_prec(PREC_ASSIGNMENT, parser);
}

static ast *unary_node(parser *p)
{
	ast *s  = NULL;
	s       = _ast_node(p->prev);
	s->left = expression(p);
	return s;
}

static ast *op_node(parser *parser)
{
	ast *s  = NULL;
	s       = _ast_node(parser->cur);
	s->left = _prefix(parser->prev)(parser);
	return s;
}

static ast *group_node(parser *parser)
{
	ast *s    = NULL;
	ast *root = NULL;

	s = _grouping(parser);

	if (_infix(parser->cur))
	{
		root       = _ast_node(parser->cur);
		root->left = s;
		return root;
	}

	return s;
}

static ast *parse_node(parser *parser)
{

	if (_infix(parser->cur))
		return op_node(parser);

	return _prefix(parser->prev)(parser);
}

static ast *parse_prec(prec_t prec, parser *parser)
{
	advance(parser);

	ast *s = NULL;

	if (!_prefix(parser->prev))
	{
		err("ERROR: Expect expression.", parser);
		return NULL;
	}
	s = parse_node(parser);

	if (!_infix(parser->cur))
		return s;
	// if (!s)
	// s = prefix(parser);

	// if (!s)
	// return prefix(parser);

	if (prec <= _prec(parser->cur))
	{
		advance(parser);
		s->right = _infix(parser->prev)(parser);
		// s->right = (parser->cur.type == TOKEN_CH_LPAREN)
		//    ? expression(parser)
		//    : _infix(parser->prev)(parser);
	}
	return s;
}

static ast *ternary_statement(parser *parser)
{
	ast *s    = NULL;
	ast *root = NULL;
	s         = expression(parser);
	consume(
	    TOKEN_CH_COLON, "Expected a `:` between ternary expressions", parser
	);
	root        = _ast_node(_token(TOKEN_CH_COLON));
	root->left  = s;
	root->right = expression(parser);
	return root;
}

static ast *_binary(parser *parser)
{
	return parse_prec((prec_t)(_prec(parser->cur) + 1), parser);
}

static ast *cast(parser *p)
{
}

static ast *_grouping(parser *parser)
{
	ast *s = NULL;
	s      = parse_prec(PREC_ASSIGNMENT, parser);
	consume(TOKEN_CH_RPAREN, "Expected a closing brace", parser);
	return s;
}

static void _append(ast **s, ast *a)
{
	ast *tmp = NULL;
	for (tmp = *s; tmp->right; tmp = tmp->right)
		;

	tmp->right       = _ast_node(_token("^"));
	tmp->right->left = a;
}

static ast *_call(parser *parser)
{
	ast *s = NULL;

	if (match(TOKEN_CH_RPAREN, parser))
		return _ast_node(_token(TOKEN_CH_RPAREN));

	s       = _ast_node(_token("^"));
	s->left = expression(parser);

	while (match(TOKEN_CH_COMMA, parser))
		_append(&s, expression(parser));

	_append(&s, _ast_node(_token(TOKEN_CH_RPAREN)));

	consume(
	    TOKEN_CH_RPAREN, "Expect closing `)` after function call", parser
	);
	return s;
}

uint8_t is_unary(token_t type)
{
	switch (type)
	{
	case TOKEN_OP_TYPE_BANG:
	case TOKEN_OP_TYPE_BITWISE_NOT:
		return 1;
	default:
		return 0;
	}
}

static uint8_t match(token_t type, parser *parser)
{
	if (type == parser->cur.type)
	{
		advance(parser);
		return 1;
	}
	return 0;
}
static void consume(token_t type, const char *expected, parser *parser)
{
	if (type == parser->cur.type)
	{
		advance(parser);
		return;
	}
	current_err(expected, parser);
}

static ast *_literal_(parser *parser)
{
	return _ast_node(parser->prev);
}

static ast *_id_(parser *parser)
{
	ast *node = NULL;
	ast *s    = NULL;
	node      = _ast_node(parser->prev);

	if (assignment_op(parser))
	{
		s        = _ast_node(parser->prev);
		s->left  = node;
		s->right = expression(parser);
		return s;
	}
	return node;
}

parser parse(const char *src, const char *file)
{
	parser parser;
	init_scanner(src);
	init_parser(&parser, file);

	while (!match(TOKEN_TYPE_EOF, &parser))
		push_ast(&parser.ast, declaration(&parser));

	return parser;
}
