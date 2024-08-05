
#ifndef _TOKEN_TYPE_H
#define _TOKEN_TYPE_H

typedef enum
{
	TOKEN_CH_LPAREN,
	TOKEN_CH_RPAREN,
	TOKEN_CH_LCURL,
	TOKEN_CH_RCURL,
	TOKEN_CH_LSQUARE,
	TOKEN_CH_RSQUARE,
	TOKEN_CH_COMMA,
	TOKEN_CH_DOT,
	TOKEN_CH_SEMI,
	TOKEN_CH_COLON,
	TOKEN_CH_TERNARY,
	TOKEN_CH_NULL_COALESCING,

	// One or two character tokens.
	TOKEN_OP_DIV,
	TOKEN_OP_MUL,
	TOKEN_OP_SUB,
	TOKEN_OP_ADD,
	TOKEN_OP_MOD,

	TOKEN_OP_REM,
	TOKEN_OP_FREE,

	TOKEN_OP_ALLOC,

	TOKEN_ALLOC_VECTOR,
	TOKEN_ALLOC_STACK,
	TOKEN_ALLOC_ARRAY,
	TOKEN_ALLOC_STR,
	TOKEN_ALLOC_INTS,
	TOKEN_ALLOC_LONGS,
	TOKEN_ALLOC_DOUBLES,
	TOKEN_ALLOC_BOOLS,

	TOKEN_OP_ASSIGN,

	TOKEN_ADD_ASSIGN,
	TOKEN_SUB_ASSIGN,
	TOKEN_MUL_ASSIGN,
	TOKEN_DIV_ASSIGN,
	TOKEN_MOD_ASSIGN,
	TOKEN_AND_ASSIGN,
	TOKEN_OR__ASSIGN,

	TOKEN_OP_CAST,
	TOKEN_OP_BANG,

	TOKEN_OP_SEQ,
	TOKEN_OP_SNE,
	TOKEN_OP_NE,
	TOKEN_OP_EQ,
	TOKEN_OP_GT,
	TOKEN_OP_GE,
	TOKEN_OP_LT,
	TOKEN_OP_LE,
	TOKEN_OP_INC,
	TOKEN_OP_DEC,

	TOKEN_OP_AND,
	TOKEN_OP_OR,
	TOKEN_SC_AND,
	TOKEN_SC_OR,
	TOKEN_LG_AND,
	TOKEN_LG_OR,

	// Literals.
	TOKEN_ID,
	TOKEN_STR,

	TOKEN_CHAR,
	TOKEN_NUMBER,

	TOKEN_TYPE_ARRAY,
	TOKEN_TYPE_VECTOR,
	TOKEN_TYPE_STACK,
	TOKEN_TYPE_TABLE,

	TOKEN_STORAGE_TYPE_NUM,
	TOKEN_STORAGE_TYPE_CHAR,
	TOKEN_STORAGE_TYPE_BOOL,
	TOKEN_STORAGE_TYPE_STR,

	TOKEN_INCLUDE,

	// Keywords.
	TOKEN_CLASS,
	TOKEN_ELSE,
	TOKEN_FUNC,
	TOKEN_FALSE,
	TOKEN_EACH,
	TOKEN_FOR,
	TOKEN_IF,
	TOKEN_SWITCH,
	TOKEN_CASE,
	TOKEN_CLOCK,
	TOKEN_READ,
	TOKEN_SQRT,
	TOKEN_FILE,
	TOKEN_PRIME,
	TOKEN_BREAK,
	TOKEN_DEFAULT,
	TOKEN_ELIF,
	TOKEN_NULL,
	TOKEN_PRINT,
	TOKEN_RETURN,

	TOKEN_PI,
	TOKEN_EULER,

	TOKEN_SUPER,
	TOKEN_THIS,
	TOKEN_TRUE,
	TOKEN_TABLE,
	TOKEN_VAR,
	TOKEN_LINE_COMMENT,
	TOKEN_NLINE_COMMENT,
	TOKEN_WHILE,

	TOKEN_ERR,
	TOKEN_EOF
} token_t;

#endif
