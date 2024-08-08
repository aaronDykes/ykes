#ifndef _PARSER_UTIL_H
#define _PARSER_UTIL_H
#include "parser_common.h"

static void error_at(token *toke, parser *parser, const char *err);
static void current_err(const char *err, parser *parser);

static void init_parser(parser *parser, const char *file);
static body class_declaration(parser *parser);
static body variable_declaration(parser *parser);
static body declaration(parser *parser);
static body statement(parser *c);
static body default_expression(parser *parser);
static ast *expression(parser *parser);

static void _append(ast **s, ast *a);
static body _block(parser *parser);
static void _if_body(parser *parser, ast_stack **stk);

static void    advance(parser *parser);
static ast    *parse_prec(prec_t prec, parser *parser);
static uint8_t is_unary(token_t type);
static uint8_t assignment_op(parser *p);

static uint8_t match(token_t type, parser *parser);
static void    consume(token_t type, const char *err, parser *parser);

static void     current_err(const char *err, parser *parser);
static void     err(const char *err, parser *parser);
static PRule   *_rule(token token);
static prec_t   _prec(token token);
static parse_fn _infix(token token);
static parse_fn _prefix(token token);

static ast *unary_node(parser *parser);

static ast *_literal_(parser *parser);
static ast *_id_(parser *parser);
static ast *dot(parser *parser);

static ast *ternary_statement(parser *parser);
static ast *cast(parser *p);

static ast *_binary(parser *parser);
static ast *_grouping(parser *parser);
static ast *group_node(parser *parser);
static ast *_call(parser *parser);

static PRule rules[] = {
    [TOKEN_CH_LPAREN]          = {group_node,           _call,             PREC_CALL      },
    [TOKEN_CH_RPAREN]          = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_CH_LCURL]           = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_CH_RCURL]           = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_CH_RSQUARE]         = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_CH_COMMA]           = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_CH_SEMI]            = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_CH_DOT]             = {NULL,                 dot,               PREC_CALL      },
    [TOKEN_OP_INC]             = {unary_node,           NULL,              PREC_TERM      },
    [TOKEN_OP_DEC]             = {unary_node,           NULL,              PREC_TERM      },
    [TOKEN_OP_SUB]             = {unary_node,           _binary,           PREC_TERM      },
    [TOKEN_OP_ADD]             = {NULL,                 _binary,           PREC_TERM      },
    [TOKEN_OP_DIV]             = {NULL,                 _binary,           PREC_FACTOR    },
    [TOKEN_OP_MOD]             = {NULL,                 _binary,           PREC_FACTOR    },
    [TOKEN_OP_MUL]             = {NULL,                 _binary,           PREC_FACTOR    },
    [TOKEN_OP_ASSIGN]          = {NULL,                 NULL,              PREC_ASSIGNMENT},
    [TOKEN_ADD_ASSIGN]         = {NULL,                 NULL,              PREC_ASSIGNMENT},
    [TOKEN_SUB_ASSIGN]         = {NULL,                 NULL,              PREC_ASSIGNMENT},
    [TOKEN_MUL_ASSIGN]         = {NULL,                 NULL,              PREC_ASSIGNMENT},
    [TOKEN_DIV_ASSIGN]         = {NULL,                 NULL,              PREC_ASSIGNMENT},
    [TOKEN_MOD_ASSIGN]         = {NULL,                 NULL,              PREC_ASSIGNMENT},
    [TOKEN_AND_ASSIGN]         = {NULL,                 NULL,              PREC_ASSIGNMENT},
    [TOKEN_OR__ASSIGN]         = {NULL,                 NULL,              PREC_ASSIGNMENT},
    [TOKEN_OP_BANG]            = {unary_node,           NULL,              PREC_TERM      },
    [TOKEN_OP_NE]              = {NULL,                 _binary,           PREC_EQUALITY  },
    [TOKEN_OP_EQ]              = {NULL,                 _binary,           PREC_EQUALITY  },
    [TOKEN_OP_GT]              = {NULL,                 _binary,           PREC_COMPARISON},
    [TOKEN_OP_GE]              = {NULL,                 _binary,           PREC_COMPARISON},
    [TOKEN_OP_LT]              = {NULL,                 _binary,           PREC_COMPARISON},
    [TOKEN_OP_LE]              = {NULL,                 _binary,           PREC_COMPARISON},
    [TOKEN_SC_AND]             = {NULL,                 NULL,              PREC_AND       },
    [TOKEN_SC_OR]              = {NULL,                 NULL,              PREC_OR        },
    [TOKEN_OP_AND]             = {NULL,                 _binary,           PREC_AND       },
    [TOKEN_OP_OR]              = {NULL,                 _binary,           PREC_OR        },
    [TOKEN_FALSE]              = {_literal_,            NULL,              PREC_NONE      },
    [TOKEN_TRUE]               = {_literal_,            NULL,              PREC_NONE      },
    [TOKEN_EACH]               = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_ID]                 = {_id_,                 NULL,              PREC_NONE      },
    [TOKEN_STR]                = {_literal_,            NULL,              PREC_NONE      },
    [TOKEN_FMT_STR]            = {_literal_,            NULL,              PREC_NONE      },
    [TOKEN_CH_TERNARY]         = {NULL,                 ternary_statement, PREC_NONE      },
    [TOKEN_CH_NULL_COALESCING] = {NULL,                 expression,        PREC_NONE      },
    [TOKEN_CHAR]               = {_literal_,            NULL,              PREC_NONE      },
    [TOKEN_NUMBER]             = {_literal_,            NULL,              PREC_NONE      },
    [TOKEN_CLASS]              = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_ELSE]               = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_FOR]                = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_LINE_COMMENT]       = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_NLINE_COMMENT]      = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_IF]                 = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_ELIF]               = {NULL,                 NULL,              PREC_OR        },
    [TOKEN_NULL]               = {_literal_,            NULL,              PREC_NONE      },
    [TOKEN_CLOCK]              = {parse_native_var_arg, NULL,              PREC_CALL      },
    [TOKEN_SQRT]               = {parse_native_var_arg, NULL,              PREC_CALL      },
    [TOKEN_PRIME]              = {parse_native_var_arg, NULL,              PREC_CALL      },
    [TOKEN_FILE]               = {parse_native_var_arg, NULL,              PREC_CALL      },
    [TOKEN_ALLOC_STR]          = {parse_native_var_arg, NULL,              PREC_NONE      },
    [TOKEN_PRINT]              = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_RETURN]             = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_SUPER]              = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_THIS]               = {_this,                NULL,              PREC_NONE      },
    [TOKEN_VAR]                = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_TYPE_ARRAY]         = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_STORAGE_TYPE_NUM]   = {cast,                 NULL,              PREC_CALL      },
    [TOKEN_STORAGE_TYPE_STR]   = {cast,                 NULL,              PREC_CALL      },
    [TOKEN_STORAGE_TYPE_CHAR]  = {cast,                 NULL,              PREC_CALL      },
    [TOKEN_STORAGE_TYPE_BOOL]  = {cast,                 NULL,              PREC_CALL      },
    [TOKEN_PI]                 = {pi,                   NULL,              PREC_NONE      },
    [TOKEN_EULER]              = {euler,                NULL,              PREC_NONE      },
    [TOKEN_WHILE]              = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_ERR]                = {NULL,                 NULL,              PREC_NONE      },
    [TOKEN_EOF]                = {NULL,                 NULL,              PREC_NONE      },
};

/*
static PRule rules[] = {

    [TOKEN_CHAR_TYPE_LCURL]  = {NULL,      NULL,    PREC_NONE       },
    [TOKEN_CHAR_TYPE_RCURL]  = {NULL,      NULL,    PREC_NONE       },
    [TOKEN_CH_LPAREN] = {_grouping, _call,   PREC_CALL       },
    [TOKEN_CH_RPAREN] = {NULL,      NULL,    PREC_NONE       },
    [TOKEN_CH_COMMA]  = {NULL,      NULL,    PREC_NONE       },
    [TOKEN_CH_SEMI]   = {NULL,      NULL,    PREC_NONE       },
    [TOKEN_CHAR_TYPE_DOT]    = {NULL,      NULL,    PREC_CALL       },
    [TOKEN_TYPE_CHAR_LITERAL]   = {_literal_, NULL,    PREC_NONE       },
    [TOKEN_TYPE_STRING_LITERAL] = {_literal_, NULL,    PREC_NONE       },
    [TOKEN_TYPE_INT_LITERAL]    = {_literal_, NULL,    PREC_NONE       },
    [TOKEN_TYPE_DOUBLE_LITERAL] = {_literal_, NULL,    PREC_NONE       },
    [TOKEN_TYPE_TRUE]           = {_literal_, NULL,    PREC_NONE       },
    [TOKEN_TYPE_FALSE]          = {_literal_, NULL,    PREC_NONE       },
    [TOKEN_OP_TYPE_ADD] = {NULL,      _binary, PREC_TERM       },
    [TOKEN_OP_TYPE_SUB] = {NULL,      _binary, PREC_TERM       },
    [TOKEN_OP_TYPE_DIV] = {NULL,      _binary, PREC_FACTOR     },
    [TOKEN_OP_TYPE_MUL] = {NULL,      _binary, PREC_FACTOR     },
    [TOKEN_OP_TYPE_MOD] = {NULL,      _binary, PREC_FACTOR     },
    [TOKEN_OP_TYPE_EQ] = {NULL,      _binary, PREC_EQUALITY   },
    [TOKEN_OP_TYPE_NE] = {NULL,      _binary, PREC_EQUALITY   },
    [TOKEN_OP_TYPE_PRE_INC]  = {NULL,      NULL,    PREC_CALL       },
    [TOKEN_OP_TYPE_PRE_DEC]  = {NULL,      NULL,    PREC_CALL       },
    [TOKEN_OP_TYPE_POST_INC] = {NULL,      _binary, PREC_DEREFERENCE},
    [TOKEN_OP_TYPE_POST_DEC] = {NULL,      _binary, PREC_DEREFERENCE},
    [TOKEN_OP_TYPE_COMPOUND_ADD] = {NULL,      NULL,    PREC_ASSIGNMENT },
    [TOKEN_OP_TYPE_COMPOUND_SUB] = {NULL,      NULL,    PREC_ASSIGNMENT },
    [TOKEN_OP_TYPE_COMPOUND_DIV] = {NULL,      NULL,    PREC_ASSIGNMENT },
    [TOKEN_OP_TYPE_COMPOUND_MUL] = {NULL,      NULL,    PREC_ASSIGNMENT },
    [TOKEN_OP_TYPE_COMPOUND_MOD] = {NULL,      NULL,    PREC_ASSIGNMENT },
    [TOKEN_OP_TYPE_LOGICAL_AND] = {NULL,      _binary, PREC_LOGIC_AND  },
    [TOKEN_OP_TYPE_LOGICAL_NOT] = {NULL,      NULL,    PREC_DEREFERENCE},
    [TOKEN_OP_TYPE_LOGICAL_OR]  = {NULL,      _binary, PREC_LOGIC_OR   },

    [TOKEN_OP_TYPE_BITWISE_AND] = {NULL,      NULL,    PREC_BITWISE_AND},
    [TOKEN_OP_TYPE_BITWISE_XOR] = {NULL,      NULL,    PREC_BITWISE_OR },
    [TOKEN_OP_TYPE_BITWISE_NOT] = {NULL,      NULL,    PREC_DEREFERENCE},
    [TOKEN_OP_TYPE_BITWISE_OR]  = {NULL,      NULL,    PREC_NONE       },

    [TOKEN_OP_TYPE_BITWISE_COMPOUND_AND] = {NULL,      NULL,    PREC_NONE },
    [TOKEN_OP_TYPE_BITWISE_COMPOUND_XOR] = {NULL,      NULL,    PREC_NONE },
    [TOKEN_OP_TYPE_BITWISE_COMPOUND_NOT] = {NULL,      NULL,    PREC_NONE },

    [TOKEN_TYPE_ID]      = {_literal_, NULL,    PREC_NONE       },
    [TOKEN_TYPE_IF]      = {NULL,      NULL,    PREC_NONE       },
    [TOKEN_TYPE_ELSE]    = {NULL,      NULL,    PREC_NONE       },
    [TOKEN_TYPE_ELSE_IF] = {NULL,      NULL,    PREC_NONE       },
    [TOKEN_TYPE_SWITCH]  = {NULL,      NULL,    PREC_NONE       },
    [TOKEN_TYPE_CASE]    = {NULL,      NULL,    PREC_NONE       },
    [TOKEN_TYPE_BREAK]   = {NULL,      NULL,    PREC_NONE       },
    [TOKEN_TYPE_RETURN]  = {NULL,      NULL,    PREC_NONE       },
    [TOKEN_TYPE_DEFAULT] = {NULL,      NULL,    PREC_NONE       },
    [TOKEN_TYPE_EOF]     = {NULL,      NULL,    PREC_NONE       },
    [TOKEN_TYPE_ERROR]   = {NULL,      NULL,    PREC_NONE       },
};

*/
#endif
