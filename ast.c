
#include "ast.h"
#include "mem.h"
#include <string.h>

char *strip_token(token *t)
{
	char *ch = NULL;
	ch       = ALLOC(t->size);
	memcpy(ch, t->start, t->size);
	ch[t->size] = '\0';
	return ch;
}

ast *_ast_node(token token)
{
	ast *a = NULL;
	a      = ALLOC(sizeof(ast));

	a->left  = NULL;
	a->right = NULL;
#ifdef _DEBUG_AST_STRUCTURE
	a->token       = token;
	a->token.start = strip_token(&token);
#else
	a->token = token;
#endif

	a->size = token.size;

	return a;
}

ast _ast_null(void)
{
	ast a;
	a.left       = NULL;
	a.right      = NULL;
	a.token.type = TOKEN_TYPE_ERROR;
	return a;
}

void push_ast(ast_stack **stk, body ast)
{
	if ((*stk)->count + 1 > (*stk)->len)
	{
		size_t size = (*stk)->len * INC;
		(*stk)->as  = realloc_ast_stack(&(*stk)->as, (*stk)->len, size);
		(*stk)->len = size;
	}
	*((*stk)->as + (*stk)->count++) = ast;
}

body *realloc_ast_stack(body **stk, size_t old_size, size_t size)
{
	if (size == 0)
	{
		FREE(*stk);
		stk = NULL;
		return NULL;
	}
	if (!*stk && size != 0)
		return ALLOC(sizeof(body) * size);

	body *realloced = NULL;
	realloced       = ALLOC(sizeof(body) * size);

	size_t new_size = size < old_size ? size : old_size;

	for (size_t i = 0; i < new_size; i++)
		*(realloced + i) = *((*stk) + i);

	FREE(*stk);
	stk = NULL;
	return realloced;
}

ast_stack *_ast_stack(size_t size)
{
	ast_stack *ast = NULL;
	ast            = ALLOC(sizeof(ast_stack));
	ast->as        = NULL;
	ast->as        = ALLOC(sizeof(body) * size);
	ast->count     = 0;
	ast->len       = size;
	return ast;
}

ast_stack parser_ast_stack(size_t size)
{
	ast_stack ast;
	ast.as    = NULL;
	ast.as    = ALLOC(sizeof(body) * size);
	ast.count = 0;
	ast.len   = size;
	return ast;
}

token _token(const char *val, token_t type)
{
	token t;
	t.start = val;
	t.size  = 1;
	t.type  = type;
	return t;
}

body _ast(void *ast, ast_t type)
{
	body b;
	b.type = type;
	b.ast  = NULL;
	b.ast  = ast;
	return b;
}
