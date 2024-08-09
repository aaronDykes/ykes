#include "chunk.h"
#include "error.h"
#include "ir.h"
#include "ir_util.h"
#include "object_math.h"
#include "parser.h"
#include <string.h>

#ifdef _DBG_VM_INSTRUCTION
#include "debug.h"
#endif

static void init_ir(ir *_ir)
{

	_ir->lookup = NULL;
	_ir->ast    = NULL;
	_ir->obj    = 0;
}

static uint8_t _literal(token_t type)
{
	switch (type)
	{
	case TOKEN_ID:
	case TOKEN_NUMBER:
	case TOKEN_CHAR:
	case TOKEN_STR:
	case TOKEN_TRUE:
	case TOKEN_FALSE:
		return 1;
	default:
		return 0;
	}
}
static uint8_t _is_identifier(token_t type)
{
	return (type == TOKEN_ID);
}

static uint8_t _value(token_t type)
{
	return _literal(type) || _is_identifier(type);
}

static uint8_t binary_op(token_t type)
{
	switch (type)
	{
	case TOKEN_OP_ADD:
	case TOKEN_OP_SUB:
	case TOKEN_OP_DIV:
	case TOKEN_OP_MUL:
	case TOKEN_OP_MOD:
		return 1;

	default:
		return 0;
	}
}
static uint8_t infix_op(token_t type)
{
	switch (type)
	{
	case TOKEN_OP_ADD:
	case TOKEN_OP_SUB:
	case TOKEN_OP_DIV:
	case TOKEN_OP_MUL:
	case TOKEN_OP_MOD:
	case TOKEN_OP_AND:
	case TOKEN_OP_OR:
	case TOKEN_OP_EQ:
	case TOKEN_OP_NE:
		return 1;

	default:
		return 0;
	}
}

static void free_tree(ast **a)
{
	if (!*a)
		return;

	ast *left  = NULL;
	ast *right = NULL;

	left  = (*a)->left;
	right = (*a)->right;

	FREE(*a);
	free_tree(&left);
	free_tree(&right);
}

static uint8_t valid_node(ast *s)
{
	if (s->left || s->right)
		return 1;

	return 0;
}

static uint8_t valid_and(ast *s)
{
	return (s->token.type == TOKEN_OP_AND);
}
static uint8_t valid_or(ast *s)
{
	return (s->token.type == TOKEN_OP_OR);
}

static uint8_t valid_logical(ast *s)
{
	return (s->token.type == TOKEN_OP_NE || s->token.type == TOKEN_OP_EQ);
}
static uint8_t valid_call(ast *s)
{
	if (s->token.type != TOKEN_CH_LPAREN)
		return 0;
	if (!s->left)
		return 0;
	if (s->left->token.type != TOKEN_ID)
		return 0;

	return 1;
}
static void emit_call(ir *c, ast **stk)
{
	_key name =
	    Key((char *)(*stk)->left->token.start, (*stk)->left->token.size);

	int arg = resolve_func(c, &name);

	if (arg == -1)
		exit_error(
		    "Call to unkown function: %s, %d\n", name.Str,
		    (*stk)->left->token.line
		);

	uint8_t argc = 0;

	for (ast *s = (*stk)->right; s && s->token.type == TOKEN_PARAM;
	     s      = s->right, argc++)
            eval_expr(c, &s->left);
}

static uint8_t evaluable(ast *ast)
{
	if (ast->right && ast->left)
		return ((ast->right->evaluated && ast->right->element.type != T_KEY
		        ) ||
		        (!ast->right->evaluated && _literal(ast->right->token.type))
		       ) &&
		       ((ast->left->evaluated && ast->left->element.type != T_KEY
		        ) ||
		        (!ast->left->evaluated && _literal(ast->left->token.type)));

	return 0;
}

static void evaluate_node(ir *_ir, ast **s)
{

	element res;
	element a = ((*s)->right->evaluated) ? ((*s)->right->element)
	                                     : create_const((*s)->right);
	element b = ((*s)->left->evaluated) ? ((*s)->left->element)
	                                    : create_const((*s)->left);

	switch ((*s)->token.type)
	{
	case TOKEN_OP_ADD:
		res = _add(&a, &b);
		break;
	case TOKEN_OP_SUB:
		res = _sub(&a, &b);
		break;
	case TOKEN_OP_DIV:
		res = _div(&a, &b);
		break;
	case TOKEN_OP_MUL:
		res = _mul(&a, &b);
		break;
	case TOKEN_OP_MOD:
		res = _mod(&a, &b);
		break;
	case TOKEN_OP_GT:
		res = _gt(&a, &b);
		break;
	case TOKEN_OP_GE:
		res = _ge(&a, &b);
		break;
	case TOKEN_OP_LT:
		res = _lt(&a, &b);
		break;
	case TOKEN_OP_LE:
		res = _le(&a, &b);
		break;
	case TOKEN_OP_EQ:
		res = _eq(&a, &b);
		break;
	case TOKEN_OP_NE:
		res = _ne(&a, &b);
		break;
	case TOKEN_OP_AND:
		res = _and(&a, &b);
		break;
	case TOKEN_OP_OR:
		res = _or(&a, &b);
		break;
	default:
		error("Invalid evaluation");
		return;
	}
	(*s)->evaluated = 1;
	(*s)->element   = res;
	FREE((*s)->left);
	FREE((*s)->right);
	(*s)->left  = NULL;
	(*s)->right = NULL;
}

static void emit_logical(ir *c, ast **a, token_t type)
{

	eval_expr(c, &(*a)->left);
	if (evaluable(*a))
	{
		evaluate_node(c, a);
		return;
	}
	if ((type == TOKEN_OP_AND) &&
	    ((*a)->left->evaluated && !(*a)->left->element.as.Bool))
		free_tree(&(*a)->right);
	else if ((type == TOKEN_OP_OR) &&
	         ((*a)->left->evaluated && (*a)->left->element.as.Bool))
		free_tree(&(*a)->right);

	eval_expr(c, &(*a)->right);
	if (evaluable(*a))
	{
		evaluate_node(c, a);
		return;
	}
}
static void emit_conditional(ir *_ir, ast **a)
{
	eval_expr(_ir, &(*a)->left);
	if (evaluable(*a))
	{
		evaluate_node(_ir, a);
		return;
	}
	eval_expr(_ir, &(*a)->right);
	if (evaluable(*a))
	{
		evaluate_node(_ir, a);
		return;
	}
}

static void set_literal(ast **a)
{
	(*a)->evaluated = 1;
	(*a)->element   = create_const(*a);
}
static void emit_numeric(ir *c, ast **a)
{
	eval_expr(c, &(*a)->left);
	if (evaluable(*a))
	{
		evaluate_node(c, a);
		return;
	}
	eval_expr(c, &(*a)->right);
	if (evaluable(*a))
	{
		evaluate_node(c, a);
		return;
	}
}

static void eval_expr(ir *_ir, ast **ast)
{
	if (!(*ast))
		return;

	if (valid_call(*ast))
	{
		emit_call(_ir, ast);
		return;
	}
	if (evaluable(*ast))
	{
		evaluate_node(_ir, ast);
		return;
	}
	if (binary_op((*ast)->token.type))
	{
		emit_numeric(_ir, ast);
		return;
	}
	if (valid_logical(*ast))
	{
		emit_conditional(_ir, ast);
		return;
	}
	if (valid_and(*ast))
	{
		emit_logical(_ir, ast, TOKEN_OP_AND);
		return;
	}
	if (valid_or(*ast))
	{
		emit_logical(_ir, ast, TOKEN_OP_OR);
		return;
	}
	if (!(*ast)->evaluated &&
	    (_literal((*ast)->token.type) || (*ast)->token.type == TOKEN_ID))
	{
		set_literal(ast);
		return;
	}
}

static void eval_statement_or_block(ir *_ir, ast_stack *stk)
{
	if (stk->as->type == AST_BLOCK)
		eval_block(_ir, &stk);
	else
		eval_statement(_ir, &stk);
}

static void eval_else_if(ir *_ir, ast_stack **stk)
{
	body *tmp = NULL;
	for (int i = 2; i < (*stk)->count; i++)
	{
		tmp = ((*stk)->as + i);
		if (tmp->type == AST_CONDITION)
		{
			eval_expr(_ir, (ast **)&tmp->ast);
			eval_statement_or_block(_ir, BLOCK(tmp));
		}
		else
			eval_statement_or_block(_ir, BLOCK(tmp));
	}
}

static void eval_if(ir *_ir, ast_stack *s)
{

	eval_expr(_ir, (ast **)&s->as->ast);
	eval_statement_or_block(_ir, BLOCK((s->as + 1)));
	eval_else_if(_ir, &s);
}

static void eval_switch(ir *_ir, ast_stack *s)
{
}

static void eval_while(ir *_ir, ast_stack *s)
{
	eval_expr(_ir, (ast **)&s->as);
	eval_statement_or_block(_ir, s + 1);
}
static void eval_for(ir *_ir, ast_stack *stk)
{
}

static void eval_func(ir *c, ast_stack *stk)
{
	int   i;
	body *tmp = NULL;
	_key  name =
	    Key((char *)AST(stk->as)->left->token.start,
	        AST(stk->as)->left->token.size);

	AST(stk->as)->evaluated = 1;
	AST(stk->as)->element   = KEY(name);

	for (i = 1; i < stk->count; i++)
	{
		tmp = stk->as + i;

		if (tmp->type != TOKEN_ID)
			goto END;

		eval_expr(c, (ast **)&tmp->ast);
	}

END:
	eval_block(c, (ast_stack **)&(stk->as + i)->ast);
}

static void eval_print(ir *_ir, ast *stk)
{
	eval_expr(_ir, &stk);
}

static uint8_t assignment(token t)
{
	switch (t.type)
	{
	case TOKEN_OP_ASSIGN:
	case TOKEN_ADD_ASSIGN:
	case TOKEN_SUB_ASSIGN:
	case TOKEN_MUL_ASSIGN:
	case TOKEN_DIV_ASSIGN:
	case TOKEN_MOD_ASSIGN:
	case TOKEN_AND_ASSIGN:
	case TOKEN_OR__ASSIGN:
		return 1;
	default:
		return 0;
	}
}

static void eval_default(ir *_ir, ast *stk)
{
	_key name = Key((char *)stk->left->token.start, stk->left->token.size);

	stk->left->evaluated = 1;
	stk->left->element   = KEY(name);

	if (assignment(stk->token))
		eval_expr(_ir, &stk);
}

static void eval_return(ir *_ir, ast *stk)
{
}

static void eval_block(ir *_ir, ast_stack **stk)
{
	body *tmp = NULL;

	for (int i = 0; i < (*stk)->count; i++)
	{
		tmp = ((*stk)->as + i);

		switch (tmp->type)
		{
		case AST_IF:
			eval_if(_ir, BLOCK(tmp));
			break;
		case AST_SWITCH:
			eval_switch(_ir, BLOCK(tmp));
			break;
		case AST_WHILE:
			eval_while(_ir, BLOCK(tmp));
			break;
		case AST_FOR:
			eval_for(_ir, BLOCK(tmp));
			break;
		case AST_PRINT:
			eval_print(_ir, AST(tmp));
			break;
		case AST_DECLARATION:
		case AST_DEF:
		case AST_STRUCT:
			eval_default(_ir, AST(tmp));
			break;
		default:
			return;
		}
	}
}

static void eval_statement(ir *_ir, ast_stack **stk)
{

	body *tmp = NULL;
	tmp       = (*stk)->as;

	switch (tmp->type)
	{
	case AST_IF:
		eval_if(_ir, BLOCK(tmp));
		break;
	case AST_SWITCH:
		eval_switch(_ir, BLOCK(tmp));
		break;
	case AST_WHILE:
		eval_while(_ir, BLOCK(tmp));
		break;
	case AST_FOR:
		eval_for(_ir, BLOCK(tmp));
		break;
	case AST_PRINT:
		eval_print(_ir, AST(tmp));
		break;
	case AST_DEF:
		eval_default(_ir, AST(tmp));
		break;
	case AST_RETURN:
		eval_return(_ir, AST(tmp));
		break;
	default:
		return;
	}
}

static void eval_ast(ir *_ir)
{
	body *tmp = NULL;

	for (int i = 0; i < _ir->ast->count; i++)
	{
		tmp = (_ir->ast->as + i);

		switch (tmp->type)
		{
		case AST_IF:
			eval_if(_ir, BLOCK(tmp));
			break;
		case AST_SWITCH:
			eval_switch(_ir, BLOCK(tmp));
			break;
		case AST_WHILE:
			eval_while(_ir, BLOCK(tmp));
			break;
		case AST_FOR:
			eval_for(_ir, BLOCK(tmp));
			break;
		case AST_PRINT:
			eval_print(_ir, AST(tmp));
			break;
		case AST_DEF:
		case AST_DECLARATION:
		case AST_STRUCT:
			eval_default(_ir, AST(tmp));
			break;
		case AST_FUNC:
			eval_func(_ir, BLOCK(tmp));
			break;
		default:
			return;
		}
	}
}

static int resolve_func(ir *_ir, _key *name)
{
	element el = find_entry(&_ir->lookup, name);

	if (el.type == T_CLOSURE)
		return el.as.Num;

	return -1;
}

static element create_const(ast *s)
{
	token t = s->token;
	switch (t.type)
	{
	case TOKEN_TRUE:
		return Bool(true);
	case TOKEN_FALSE:
		return Bool(false);
	case TOKEN_NUMBER:
		return Num(atoll(t.start));
	case TOKEN_STR:
		return String(t.start, t.size);
	case TOKEN_CHAR:
		return Char(*t.start);
	case TOKEN_ID:
		return KeyObj((char *)t.start, t.size);

	default:
		return Null();
	}
}

ir gen_ir(const char *src, const char *file)
{

	ir _ir;
	init_ir(&_ir);

	parser p = parse(src, file);
	_ir.ast  = p.ast;
	_ir.obj  = p._obj;

	eval_ast(&_ir);

#ifdef DBG_CODE_GEN
	for (int i = 0; i < c.func->ch.ip.count; i++)
		disassemble_instruction(&c.func->ch, i);
	for (stack *s = c.func->ch.constants; s < s->top; s++)
		print(s->as);
#endif

	return _ir;
}
