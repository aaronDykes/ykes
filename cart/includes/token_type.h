
#ifndef _TOKEN_TYPE_H
#define _TOKEN_TYPE_H

enum
{

    TOKEN_CH_LPAREN,
    TOKEN_CH_RPAREN,
    TOKEN_CH_LCURL,
    TOKEN_CH_RCURL,
    TOKEN_CH_COMMA,
    TOKEN_CH_DOT,
    TOKEN_CH_SEMI,
    TOKEN_CH_COLON,

    // One or two character tokens.
    TOKEN_OP_DIV,
    TOKEN_OP_MUL,
    TOKEN_OP_SUB,
    TOKEN_OP_ADD,
    TOKEN_OP_MOD,

    TOKEN_OP_ASSIGN,

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
    TOKEN_INT,
    TOKEN_DOUBLE,
    TOKEN_BTYE,
    TOKEN_LINT,
    TOKEN_LLINT,

    // Keywords.
    TOKEN_CLASS,
    TOKEN_ELSE,
    TOKEN_FUNC,
    TOKEN_FALSE,
    TOKEN_FOR,
    TOKEN_IF,
    TOKEN_SWITCH,
    TOKEN_CASE,
    TOKEN_CLOCK,
    TOKEN_SQRT,
    TOKEN_PRIME,
    TOKEN_BREAK,
    TOKEN_DEFAULT,
    TOKEN_ELIF,
    TOKEN_NULL,
    TOKEN_PRINT,
    TOKEN_RETURN,
    TOKEN_SUPER,
    TOKEN_THIS,
    TOKEN_TRUE,
    TOKEN_VAR,
    TOKEN_LINE_COMMENT,
    TOKEN_NLINE_COMMENT,
    TOKEN_WHILE,

    TOKEN_ERR,
    TOKEN_EOF
};

#endif