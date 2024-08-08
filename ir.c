#include "chunk.h"
#include "ir.h"
#include "ir_util.h"
#include "object_math.h"
#include "parser.h"
#include <string.h>

#ifdef _DBG_VM_INSTRUCTION
#include "debug.h"
#endif
#define UPPER(x) ((uint8_t)((x >> 8) & 0xFF))
#define LOWER(x) ((uint8_t)(x & 0xFF))

static counter init_counter(void)
{
	counter c;
	c.include = 0;
	c.local   = 0;
	c.scope   = 0;
	c.upvalue = 0;
	c.obj     = 0;
	return c;
}
static void init_compiler(compiler *c, compiler_t type, key name)
{
	c->ast    = NULL;
	c->base   = NULL;
	c->lookup = NULL;
	c->prev   = NULL;
	c->func   = NULL;

	c->func        = _function(name);
	c->state.count = init_counter();
	c->state.flags = 0;
	c->state.type  = type;
}

static uint8_t _literal(token_t type)
{
	switch (type)
	{
	case TOKEN_TYPE_ID:
	case TOKEN_TYPE_INT_LITERAL:
	case TOKEN_TYPE_DOUBLE_LITERAL:
	case TOKEN_TYPE_STRING_LITERAL:
	case TOKEN_TYPE_CHAR_LITERAL:
	case TOKEN_TYPE_TRUE:
	case TOKEN_TYPE_FALSE:
		return 1;
	default:
		return 0;
	}
}
static uint8_t _is_identifier(token_t type)
{
	return (type == TOKEN_TYPE_ID || type == TOKEN_TYPE_STRUCT_ID);
}

static uint8_t _value(token_t type)
{
	return _literal(type) || _is_identifier(type);
}

static uint8_t binary_op(token_t type)
{
	switch (type)
	{
	case TOKEN_OP_TYPE_ADD:
	case TOKEN_OP_TYPE_SUB:
	case TOKEN_OP_TYPE_DIV:
	case TOKEN_OP_TYPE_MUL:
	case TOKEN_OP_TYPE_MOD:
		return 1;

	default:
		return 0;
	}
}

static uint8_t valid_node(ast *s)
{
	if (s->left || s->right)
		return 1;

	return 0;
}

static uint8_t valid_expression(ast *s)
{
	if (binary_op(s->token.type))
		return valid_node(s);
	return 0;
}

static uint8_t valid_and(ast *s)
{
	return (s->token.type == TOKEN_OP_TYPE_LOGICAL_AND);
}
static uint8_t valid_or(ast *s)
{
	return (s->token.type == TOKEN_OP_TYPE_LOGICAL_OR);
}

static uint8_t valid_logical(ast *s)
{
	return (
	    s->token.type == TOKEN_OP_TYPE_NE || s->token.type == TOKEN_OP_TYPE_EQ
	);
}
static uint8_t valid_call(ast *s)
{
	if (s->token.type != TOKEN_CH_LPAREN)
		return 0;
	if (!s->left)
		return 0;
	if (s->left->token.type != TOKEN_TYPE_ID)
		return 0;

	return 1;
}
static void emit_call(compiler *c, ast **stk)
{
	key name = _key((char *)(*stk)->left->token.start, (*stk)->left->size);

	int arg = resolve_func(c, name.key);

	if (arg == -1)
		exit_error(
		    "Call to unkown function: %s, %d\n", name.val, c->state.line
		);

	uint8_t argc = 0;

	emit_bytes(c, OP_GET_OBJ, (uint8_t)arg);

	for (ast *s = (*stk)->right; s && s->token.type == TOKEN_TYPE_PARAM;
	     s      = s->right, argc++)
            eval_expr(c, &s->left);

	emit_bytes(c, OP_CALL, argc);
}

static uint8_t evaluable(ast *ast)
{
	if (ast->right && ast->left)
	{
		if (_literal(ast->right->token.type) &&
		    _literal(ast->left->token.type))
			return 1;
	}

	return 0;
}

static void evaluate_node(compiler *compiler, ast **s)
{

	element a = create_const((*s)->right);
	element b = create_const((*s)->left);
	value   res;

	switch ((*s)->token.type)
	{
	case TOKEN_OP_TYPE_ADD:
		res = _add(a._value, b._value);
		break;
	case TOKEN_OP_TYPE_SUB:
		res = _sub(a._value, b._value);
		break;
	case TOKEN_OP_TYPE_DIV:
		res = _div(a._value, b._value);
		break;
	case TOKEN_OP_TYPE_MUL:
		res = _mul(a._value, b._value);
		break;
	case TOKEN_OP_TYPE_MOD:
		res = _mod(a._value, b._value);
		break;
	case TOKEN_OP_TYPE_EQ:
		res = _eq(a._value, b._value);
		break;
	case TOKEN_OP_TYPE_NE:
		res = _ne(a._value, b._value);
		break;
	case TOKEN_OP_TYPE_LOGICAL_AND:
		res = _and(a._value, b._value);
		break;
	case TOKEN_OP_TYPE_LOGICAL_OR:
		res = _or(a._value, b._value);
		break;
	default:
		error("Invalid evaluation");
		return;
	}
	emit_const_element(compiler, OBJ(res));
	*s = NULL;
	s  = NULL;
}

static void emit_expression(compiler *compiler, ast **ast)
{
	if (evaluable(*ast))
	{
		evaluate_node(compiler, ast);
		return;
	}
	if ((*ast)->right)
		emit_const_token(compiler, (*ast)->right);
	if ((*ast)->left)
		emit_const_token(compiler, (*ast)->left);

	switch ((*ast)->token.type)
	{
	case TOKEN_OP_TYPE_ADD:
		emit_byte(compiler, OP_ADD);
		break;
	case TOKEN_OP_TYPE_SUB:
		emit_byte(compiler, OP_SUB);
		break;
	case TOKEN_OP_TYPE_DIV:
		emit_byte(compiler, OP_DIV);
		break;
	case TOKEN_OP_TYPE_MUL:
		emit_byte(compiler, OP_MUL);
		break;
	case TOKEN_OP_TYPE_MOD:
		emit_byte(compiler, OP_MOD);
		break;
	case TOKEN_OP_TYPE_NE:
		emit_byte(compiler, OP_NE);
		break;
	case TOKEN_OP_TYPE_EQ:
		emit_byte(compiler, OP_EQ);
		break;
	case TOKEN_OP_TYPE_LOGICAL_AND:
		emit_byte(compiler, OP_AND);
		break;
	case TOKEN_OP_TYPE_LOGICAL_OR:
		emit_byte(compiler, OP_OR);
		break;
	default:
		error("Invalid evaluation");
	}

	*ast = NULL;
	ast  = NULL;
}

static void emit_numeric(compiler *c, ast **a)
{
	if (evaluable(*a))
	{
		evaluate_node(c, a);
		return;
	}

	eval_expression(c, &(*a)->right);
	eval_expression(c, &(*a)->left);
	emit_expression(c, a);
}

static void emit_logical(compiler *c, ast **a, uint8_t code)
{
	if (evaluable(*a))
	{
		evaluate_node(c, a);
		return;
	}
	eval_expression(c, &(*a)->left);

	emit_byte(c, code);
	emit_bytes(
	    c, UPPER(c->func->ch.expr_index.count),
	    LOWER(c->func->ch.expr_index.count)
	);

	emit_byte(c, OP_POP);
	eval_expression(c, &(*a)->right);
	emit_expression(c, a);
}
static void emit_conditional(compiler *compiler, ast **a)
{
	if (evaluable(*a))
	{
		evaluate_node(compiler, a);
		return;
	}
	eval_expression(compiler, &(*a)->left);
	eval_expression(compiler, &(*a)->right);
	emit_expression(compiler, a);
}

static void emit_literal(compiler *c, ast **a)
{
	emit_const_token(c, *a);
	*a = NULL;
	a  = NULL;
}

static void eval_expression(compiler *compiler, ast **ast)
{
	if (!(*ast))
		return;

	if (valid_call(*ast))
	{
		emit_call(compiler, ast);
		return;
	}
	if (valid_expression(*ast))
	{
		emit_numeric(compiler, ast);
		return;
	}
	if (valid_logical(*ast))
	{
		emit_conditional(compiler, ast);
		return;
	}
	if (valid_and(*ast))
	{
		emit_logical(compiler, ast, OP_JMPL_F);
		return;
	}
	if (valid_or(*ast))
	{
		emit_logical(compiler, ast, OP_JMPL_T);
		return;
	}
	if (_literal((*ast)->token.type))
	{
		emit_literal(compiler, ast);
		return;
	}
}
static void eval_expr(compiler *compiler, ast **s)
{
	eval_expression(compiler, s);
	compiler->func->ch.expr_index
	    .bytes[compiler->func->ch.expr_index.count++] =
	    compiler->func->ch.ip.count;
}

static void eval_statement_or_block(compiler *compiler, ast_stack *stk)
{
	if (stk->as.type == AST_BLOCK)
		eval_block(compiler, stk->as.block);
	else
		eval_statement(compiler, stk);
}

static void eval_else_if(compiler *compiler, ast_stack *stk)
{
	ast_stack *tmp = NULL;
	for (int i = 2; i < stk->count; i++)
	{
		tmp = (stk + i);
		if (tmp->as.type == AST_CONDITION)
		{
			eval_expr(compiler, &tmp->as.ast);
			int jmp = emit_jmp(compiler, OP_JMPF);
			emit_byte(compiler, OP_POP);
			eval_statement_or_block(compiler, tmp);
			emit_byte(compiler, OP_JMPL);
			emit_bytes(
			    compiler, UPPER(compiler->func->ch.jmp_index.count),
			    LOWER(compiler->func->ch.jmp_index.count)
			);
			patch_jmp(compiler, jmp);
		}
		else
			eval_statement_or_block(compiler, tmp);
	}
}
static void end_scope(compiler *c)
{
	if (c->state.count.scope > 0 &&
	    c->locals[c->state.count.local - 1].scope > c->state.count.scope)
		emit_bytes(c, OP_NPOP, c->state.count.local);

	while (c->state.count.scope > 0 &&
	       c->locals[--c->state.count.local].scope > c->state.count.scope)
		;

	c->state.count.scope--;
}

static void eval_if(compiler *compiler, ast_stack *stk)
{

	compiler->state.count.scope++;
	eval_expr(compiler, &stk->as.ast);
	int exit = emit_jmp(compiler, OP_JMPF);

	emit_byte(compiler, OP_POP);
	eval_statement_or_block(compiler, stk + 1);
	int jmp = emit_jmp(compiler, OP_JMP);

	patch_jmp(compiler, exit);

	emit_byte(compiler, OP_POP);
	eval_else_if(compiler, stk);

	compiler->func->ch.jmp_index.bytes[compiler->func->ch.jmp_index.count++] =
	    compiler->func->ch.ip.count;
	patch_jmp(compiler, jmp);
	end_scope(compiler);
}

static void eval_switch(compiler *compiler, ast_stack *stk)
{
	compiler->state.count.scope++;
	end_scope(compiler);
}

static void eval_while(compiler *compiler, ast_stack *stk)
{
	compiler->state.count.scope++;
	int loop = compiler->func->ch.ip.count;
	eval_expr(compiler, &stk->as.ast);
	int exit = emit_jmp(compiler, OP_JMPF);

	eval_statement_or_block(compiler, stk + 1);
	emit_loop(compiler, loop);

	patch_jmp(compiler, exit);
	end_scope(compiler);
}
static void eval_for(compiler *compiler, ast_stack *stk)
{
}

static void eval_func(compiler *c, ast_stack *stk)
{

	function  *func = NULL;
	ast_stack *tmp  = NULL;
	compiler   co;

	if (stk->as.type == AST_FUNC_FD)
		return;

	element name = create_const(stk->as.ast);

	init_compiler(&co, _FUNCTION, name._key);

	int arg = resolve_func(c, name._key.key);

	uint8_t arity = 0;
	for (int i = 1; i < stk->count; i++)
	{
		tmp = stk + i;

		switch (tmp->as.type)
		{
		case AST_PARAM:
			arity++;
			break;
		default:
			goto END;
		}
	}
END:
	co.func->arity = arity;
	co.ast         = (stk + arity + 1)->as.block;

	co.state.count.scope++;
	co.prev = c;
	eval_ast(&co);
	end_scope(&co);

	emit_bytes(c, OP_SET_OBJ, arg);
	emit_byte(c, push_constant(&c->func->ch, GEN_OBJ(co.func, T_CLOSURE)));
}

static void eval_print(compiler *compiler, ast *stk)
{
	eval_expr(compiler, &stk);
	emit_byte(compiler, OP_PRINT);
}

static void emit_id_resolution(compiler *compiler, key name)
{

	int     arg = 0;
	uint8_t get = OP_GET_GLOBAL;

	if ((arg = resolve_local(compiler, name)) != -1)
		get = OP_GET_LOCAL;
	else
		arg = push_constant(&compiler->func->ch, KEY(name));

	emit_bytes(compiler, get, arg);
}

static void eval_default(compiler *compiler, ast *stk)
{
	key name = _key((char *)stk->left->token.start, stk->left->size);

	int     arg = 0;
	uint8_t get = OP_GET_GLOBAL, set = OP_SET_GLOBAL;

	if ((arg = resolve_func(compiler, name.key)) != -1)
	{
		emit_call(compiler, &stk);
		return;
	}
	if ((arg = resolve_local(compiler, name)) != -1)
	{
		get = OP_GET_LOCAL;
		set = OP_SET_LOCAL;
	}
	else
		arg = push_constant(&compiler->func->ch, KEY(name));

	if (stk->token.type == TOKEN_OP_TYPE_MOV)
	{
		eval_expr(compiler, &stk);
		emit_bytes(compiler, set, arg);
	}
	else
		emit_bytes(compiler, get, arg);
}

static void eval_dec(compiler *compiler, ast *stk)
{

	key name = _key((char *)stk->left->token.start, stk->left->size);

	int arg = parse_var(compiler, name);

	uint8_t set = OP_GLOBAL_DEF;
	if (arg == -1)
	{
		arg = resolve_local(compiler, name);
		set = OP_SET_LOCAL;
	}

	if (stk->token.type == TOKEN_OP_TYPE_MOV)
		eval_expr(compiler, &stk->right);

	emit_bytes(compiler, set, arg);
}

static void eval_block(compiler *compiler, ast_stack *ast)
{
	ast_stack *tmp = NULL;

	compiler->state.count.scope++;

	for (int i = 0; i < ast->count; i++)
	{
		tmp                  = (ast + i);
		compiler->state.line = tmp->as.line;

		switch (tmp->as.type)
		{
		case AST_IF:
			eval_if(compiler, tmp->as.block);
			break;
		case AST_SWITCH:
			eval_switch(compiler, tmp->as.block);
			break;
		case AST_WHILE:
			eval_while(compiler, tmp->as.block);
			break;
		case AST_FOR:
			eval_for(compiler, tmp->as.block);
			break;
		case AST_PRINT:
			eval_print(compiler, tmp->as.ast);
			break;
		case AST_DEF:
			eval_default(compiler, tmp->as.ast);
			break;
		default:
			return;
		}
	}
	compiler->state.count.scope--;
}

static void eval_statement(compiler *compiler, ast_stack *ast)
{

	compiler->state.line = ast->as.line;
	switch (ast->as.type)
	{

	case AST_IF:
		eval_if(compiler, ast->as.block);
		break;
	case AST_SWITCH:
		eval_switch(compiler, ast->as.block);
		break;
	case AST_WHILE:
		eval_while(compiler, ast->as.block);
		break;
	case AST_FOR:
		eval_for(compiler, ast->as.block);
		break;
	case AST_PRINT:
		eval_print(compiler, ast->as.ast);
		break;
	case AST_DEF:
		eval_default(compiler, ast->as.ast);
		break;
	default:
		return;
	}
}

static void eval_ast(compiler *compiler)
{
	ast_stack *tmp = NULL;

	for (int i = 0; i < compiler->ast->count; i++)
	{
		tmp                  = (compiler->ast + i);
		compiler->state.line = tmp->as.line;

		switch (tmp->as.type)
		{
		case AST_IF:
			eval_if(compiler, tmp->as.block);
			break;
		case AST_SWITCH:
			eval_switch(compiler, tmp->as.block);
			break;
		case AST_WHILE:
			eval_while(compiler, tmp->as.block);
			break;
		case AST_FOR:
			eval_for(compiler, tmp->as.block);
			break;
		case AST_PRINT:
			eval_print(compiler, tmp->as.ast);
			break;
		case AST_DEF:
			eval_default(compiler, tmp->as.ast);
			break;
		case AST_DECLARATION:
			eval_dec(compiler, tmp->as.ast);
			break;
		case AST_FUNC:
			eval_func(compiler, tmp->as.block);
			break;
		default:
			return;
		}
	}
	emit_byte(compiler, OP_RETURN);
}

static void emit_byte(compiler *compiler, uint8_t op_code)
{
	write_chunk(&compiler->func->ch, op_code, compiler->state.line);
}
static void emit_bytes(compiler *compiler, uint8_t op_code1, uint8_t op_code2)
{
	write_chunk(&compiler->func->ch, op_code1, compiler->state.line);
	write_chunk(&compiler->func->ch, op_code2, compiler->state.line);
}

static uint8_t id_cmp(key a, key b)
{
	return a.key == b.key;
}

static void add_local(compiler *c, key local)
{
	if (c->state.count.local > LOCAL_COUNT)
		exit_error("Local variable count exeeds maximum allowance of 255");

	c->locals[c->state.count.local].scope  = c->locals->scope;
	c->locals[c->state.count.local++].name = local;
}
static void declare_var(compiler *c, key name)
{
	if (c->state.count.scope == 0)
		return;

	if (resolve_local(c, name) != -1)
		exit_error("Duplicate variable identifiers in scope");

	add_local(c, name);
}
static int parse_var(compiler *c, key name)
{
	declare_var(c, name);

	if (c->state.count.scope > 0)
		return -1;

	return push_constant(&c->func->ch, KEY(name));
}

static int resolve_local(compiler *c, key name)
{
	for (int i = c->state.count.local - 1; i >= 0; i--)
		if (id_cmp(c->locals[i].name, name))
			return i;
	return -1;
}
static int resolve_func(compiler *compiler, Long name)
{
	element el = find_entry(&compiler->lookup, name);

	if (el.type != T_NULL && el.type == T_CLOSURE)
		return el._value.as._number;

	return -1;
}

static element create_const(ast *s)
{
	token t = s->token;
	switch (t.type)
	{
	case TOKEN_TYPE_TRUE:
		return OBJ(_bool(true));
	case TOKEN_TYPE_FALSE:
		return OBJ(_bool(false));
	case TOKEN_TYPE_INT_LITERAL:
		return OBJ(_number(atoll(t.start)));
	case TOKEN_TYPE_DOUBLE_LITERAL:
		return OBJ(_number(strtod(t.start, NULL)));
	case TOKEN_TYPE_STRING_LITERAL:
		return OBJ(_string(t.start, s->size));
	case TOKEN_TYPE_CHAR_LITERAL:
		return OBJ(_char(*t.start));
	case TOKEN_TYPE_ID:
		return KEY(_key((char *)t.start, s->size));

	default:
		return _NULL_();
	}
}

static void emit_const_element(compiler *compiler, element el)
{
	if (el.type == T_KEY)
	{
		emit_id_resolution(compiler, el._key);
		return;
	}
	int arg = push_constant(&compiler->func->ch, el);
	emit_bytes(compiler, OP_CONSTANT, (uint8_t)arg);
}
static void emit_const_token(compiler *compiler, ast *s)
{
	element el = create_const(s);
	if (el.type == T_KEY)
	{
		emit_id_resolution(compiler, el._key);
		return;
	}
	int arg = push_constant(&compiler->func->ch, el);
	emit_bytes(compiler, OP_CONSTANT, (uint8_t)arg);
}

static void patch_jmp(compiler *compiler, int index)
{
	int jmp = compiler->func->ch.ip.count - index - 2;

	compiler->func->ch.ip.bytes[index]     = UPPER(jmp);
	compiler->func->ch.ip.bytes[index + 1] = LOWER(jmp);
}

static void emit_loop(compiler *compiler, int index)
{
	int offset = compiler->func->ch.ip.count - index;
	emit_byte(compiler, OP_LOOP);
	emit_bytes(compiler, UPPER(offset), LOWER(offset));
}
static int emit_jmp(compiler *compiler, uint8_t op_code)
{
	emit_byte(compiler, op_code);
	emit_bytes(compiler, 0xFF, 0xFF);
	return compiler->func->ch.ip.count - 2;
}

function *compile(const char *src, const char *file)
{

	compiler c;
	init_compiler(&c, _SCRIPT, _key("script", 6));

	c.base            = &c;
	parser p          = parse(src, file);
	c.ast             = p.ast;
	c.lookup          = p.lookup;
	c.state.count.obj = p._obj;

	eval_ast(&c);

#ifdef DBG_CODE_GEN
	for (int i = 0; i < c.func->ch.ip.count; i++)
		disassemble_instruction(&c.func->ch, i);
	for (stack *s = c.func->ch.constants; s < s->top; s++)
		print(s->as);
#endif

	return c.func;
}
