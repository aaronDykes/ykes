#ifndef _PARSER_COMMON_H
#define _PARSER_COMMON_H
#include "ast.h"
#include "table.h"

typedef struct parser parser;
typedef struct PRule  PRule;
typedef ast *(*parse_fn)(parser *);

typedef enum
{
	PREC_NONE,
	PREC_ASSIGNMENT, // =
	PREC_OR,         // or
	PREC_AND,        // and
	PREC_EQUALITY,   // == !=
	PREC_COMPARISON, // < > <= >=
	PREC_TERM,       // + -
	PREC_FACTOR,     // * /
	PREC_UNARY,      // ! -
	PREC_CALL,       // . ()
	PREC_PRIMARY

} prec_t;

struct PRule
{
	parse_fn prefix;
	parse_fn infix;
	prec_t   prec;
};

struct parser
{
	ast_stack   ast;
	token       cur;
	token       prev;
	table      *lookup;
	uint16_t    _obj;
	const char *file;
};
#endif
