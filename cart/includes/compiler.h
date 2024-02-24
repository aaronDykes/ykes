#ifndef _YKES_COMPILER_H
#define _YKES_COMPILER_H
#include "chunk.h"
#include "scanner.h"

struct Parser
{
    token cur;
    token pre;
    bool had_err;
    bool fuck;
};

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

} Precedence;

typedef void (*parse_fn)();

struct parse_rule
{
    parse_fn prefix;
    parse_fn infix;
    Precedence prec;
};

typedef struct parse_rule PRule;
typedef struct Parser Parser;

bool compile(const char *src, Chunk ch);
#endif