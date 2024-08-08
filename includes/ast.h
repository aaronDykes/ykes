#ifndef _AST_H
#define _AST_H
#include "scanner.h"
#define _MIN_BLOCK_SIZE 16

typedef struct ast       ast;
typedef struct body      body;
typedef struct ast_stack ast_stack;

#define AST(bod)   ((ast *)bod.ast);
#define BLOCK(bod) ((ast_stack *)bod.ast);

typedef enum
{

	AST_CONDITION,
	AST_BLOCK,

	AST_FUNC,
	AST_FUNC_FD,
	AST_FUNC_ID,
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

	AST_DECLARATION,

} ast_t;

struct ast
{
	token token;
	ast  *left;
	ast  *right;
};

struct body
{
	ast_t type;
	void *ast;
};

struct ast_stack
{
	body    *as;
	uint16_t len;
	uint16_t count;
};

char      *strip_token(token *t);
ast       *_ast_node(token token);
ast_stack *_ast_stack(size_t size);
ast_stack  parser_ast_stack(size_t size);
body      *realloc_ast_stack(body **stk, size_t old_size, size_t size);
token      _token(const char *val);
body       _ast(void *ast, ast_t type);
void       push_ast(ast_stack **stk, body ast);

#endif
