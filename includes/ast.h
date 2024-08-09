#ifndef _AST_H
#define _AST_H
#include "object.h"
#include "scanner.h"
#define MIN_BLOCK_SIZE 8

typedef struct ast       ast;
typedef struct body      body;
typedef struct ast_stack ast_stack;

#define AST(bod)   ((ast *)bod->ast)
#define BLOCK(bod) ((ast_stack *)bod->ast)

typedef enum
{

	AST_CONDITION,
	AST_BLOCK,

	AST_STRUCT,
	AST_ID,
	AST_FUNC,
	AST_PARAM,

	AST_IF,
	AST_ELSE_IF,
	AST_ELSE,

	AST_SWITCH,
	AST_CASE,
	AST_DEFAULT,

	AST_DO,
	AST_WHILE,

	AST_TERNARY,
	AST_FOR,
	AST_PRINT,
	AST_DEF,
	AST_RETURN,

	AST_DECLARATION,

} ast_t;

struct ast
{
	uint8_t evaluated;
	union
	{
		token   token;
		element element;
	};

	ast *left;
	ast *right;
};

struct body
{
	ast_t    type;
	uint16_t line;
	void    *ast;
};

struct ast_stack
{
	body    *as;
	uint16_t len;
	uint16_t count;
};

token      token_cpy(token src);
char      *strip_token(token *t);
ast       *_ast_node(token token);
ast_stack *_ast_stack(size_t size);
ast_stack  parser_ast_stack(size_t size);
body      *realloc_ast_stack(body **stk, size_t old_size, size_t size);
token      _token(const char *val, token_t type);
body       _ast(void *ast, ast_t type);
void       push_ast(ast_stack **stk, body ast);

#endif
