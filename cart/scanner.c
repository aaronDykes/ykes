#include "scanner.h"
#include <stdlib.h>

static token make_token(token_type t);
static token err_token(const char *err);

static token string();
static token number();
static token id();
static token character();

static token_type id_type();
static token_type check_keyword(int start, int end, const char *str, token_type t);

static char next();
static char advance();
static char peek(int n);

static void skip();
static void nskip(int n);

static bool is_space();
static bool match(char expected);
static bool end();
static bool digit(char c);
static bool alpha(char c);

static void skip_line_comment();
static void skip_multi_line_comment();
static token skip_comment();
static void skip_whitespace();

void init_scanner(const char *src)
{
    scan.start = src;
    scan.current = src;
    scan.line = 1;
}
token scan_token()
{
    skip_whitespace();
    scan.start = scan.current;

    if (end())
        return make_token(TOKEN_EOF);

    char c = advance();

    if (alpha(c))
        return id();
    if (digit(c))
        return number();

    switch (c)
    {
    case '(':
        return make_token(TOKEN_CH_LPAREN);
    case ')':
        return make_token(TOKEN_CH_RPAREN);
    case '{':
        return make_token(TOKEN_CH_LCURL);
    case '}':
        return make_token(TOKEN_CH_RCURL);
    case ',':
        return make_token(TOKEN_CH_COMMA);
    case '.':
        return make_token(TOKEN_CH_DOT);
    case ';':
        return make_token(TOKEN_CH_SEMI);
    case ':':
        return make_token(TOKEN_CH_COLON);
    case '/':
        if (next() == '/' || next() == '*')
            return skip_comment();
        return make_token(TOKEN_OP_DIV);
    case '*':
        return make_token(TOKEN_OP_MUL);
    case '-':
        return make_token(TOKEN_OP_SUB);
    case '+':
        return make_token(TOKEN_OP_ADD);
    case '%':
        return make_token(TOKEN_OP_MOD);
    case '!':
        return make_token(match('=') ? TOKEN_OP_NE : TOKEN_OP_BANG);
    case '=':
        return make_token(match('=') ? TOKEN_OP_EQ : TOKEN_OP_ASSIGN);
    case '>':
        return make_token(match('=') ? TOKEN_OP_GT : TOKEN_OP_GE);
    case '<':
        return make_token(match('=') ? TOKEN_OP_LT : TOKEN_OP_LE);
    case '\'':
        return character();
    case '"':
        return string();
    }
    return err_token("ERROR: invalid token");
}

static token make_token(token_type t)
{
    token toke;
    toke.start = scan.start;
    toke.line = scan.line;
    toke.type = t;
    toke.size = (int)(scan.current - scan.start);
    return toke;
}
static token err_token(const char *err)
{
    token toke;
    toke.start = err;
    toke.line = scan.line;
    toke.type = TOKEN_ERR;
    toke.size = strlen(err);
    return toke;
}

static token string()
{
    while (next() != '"' && !end())
    {
        if (next() == '\n')
            scan.line++;

        skip();
    }

    if (end())
        return err_token("Unterminated string");

    skip();
    return make_token(TOKEN_STR);
}
static token number()
{
    bool is_whole = true;

    while (digit(next()))
        skip();
    if (next() == '.' && digit(peek(1)))
    {
        skip();
        while (digit(next()))
            skip();
        is_whole = false;
    }

    long long int val = atoll(scan.start);
    int type = (val < INT32_MAX) ? TOKEN_INT : TOKEN_LLINT;
    return is_whole ? make_token(type) : make_token(TOKEN_DOUBLE);
}
static token id()
{
    while (digit(next()) || alpha(next()))
        skip();

    return make_token(id_type());
}
static token character()
{
    nskip(3);
    return make_token(TOKEN_CHAR);
}

static token_type id_type()
{
    switch (*scan.start)
    {
    case 'a':
        return check_keyword(1, 2, "nd", TOKEN_OP_AND);
    case 'b':
        return check_keyword(1, 3, "yte", TOKEN_BTYE);
    case 'c':
        return check_keyword(1, 4, "lass", TOKEN_CLASS);
    case 'e':
        return check_keyword(1, 3, "lse", TOKEN_ELSE);
    case 'f':
        if (scan.current - scan.start > 1)
            switch (scan.start[1])
            {
            case 'o':
                return check_keyword(2, 1, "r", TOKEN_FOR);
            case 'a':
                return check_keyword(2, 3, "lse", TOKEN_FALSE);
            }
    case 'i':
        return check_keyword(1, 1, "f", TOKEN_IF);
    case 'l':
        if (scan.current - scan.start > 1)
            switch (scan.start[1])
            {
            case 'l':
                return check_keyword(2, 3, "int", TOKEN_LLINT);
            case 'i':
                return check_keyword(2, 2, "nt", TOKEN_LINT);
            }
    case 'n':
        return check_keyword(1, 3, "ull", TOKEN_NULL);
    case 'o':
        return check_keyword(1, 1, "r", TOKEN_OP_OR);
    case 'p':
        return check_keyword(1, 4, "rint", TOKEN_PRINT);
    case 'r':
        return check_keyword(1, 5, "eturn", TOKEN_RETURN);
    case 's':
        return check_keyword(1, 4, "uper", TOKEN_SUPER);
    case 't':
        if (scan.current - scan.start > 1)
            switch (scan.start[1])
            {
            case 'h':
                return check_keyword(2, 2, "is", TOKEN_THIS);
            case 'r':
                return check_keyword(2, 2, "ue", TOKEN_TRUE);
            }
    case 'v':
        return check_keyword(1, 2, "ar", TOKEN_VAR);
    case 'w':
        return check_keyword(1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_ID;
}
static token_type check_keyword(int start, int end, const char *str, token_type t)
{

    if (((int)(scan.current - scan.start) == start + end) && memcmp(scan.start + start, str, end) == 0)
        return t;

    return TOKEN_ID;
}

static char next()
{
    return *scan.current;
}
static char advance()
{
    return *scan.current++;
}
static char peek(int n)
{
    return scan.current[n];
}

static void skip()
{
    scan.current++;
}
static void nskip(int n)
{
    for (int i = 0; i < n; i++)
        skip();
}

static bool is_space()
{
    return next() == ' ' ||
           next() == '\t' ||
           next() == '\r' ||
           next() == '\n';
}
static bool match(char expected)
{
    if (next() != expected || end())
        return false;
    scan.current++;
    return true;
}
static bool end()
{
    return *scan.current == '\0';
}
static bool digit(char c)
{
    return (c >= '0' && c <= '9');
}
static bool alpha(char c)
{
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c == '_');
}

static void skip_line_comment()
{
    for (; *scan.current != '\n'; skip())
        ;

    if (!end())
        scan.line++;

    skip();
}
static void skip_multi_line_comment()
{
    skip();
    for (; !end(); skip())
        if (*scan.current == '/' && scan.current[1] == '*')
            skip_multi_line_comment();
        else if (*scan.current == '\n')
            scan.line++;
        else if (*scan.current == '*' && scan.current[1] == '/')
            break;

    nskip(2);
}
static token skip_comment()
{
    int type = TOKEN_LINE_COMMENT;
    if (match('/'))
        skip_line_comment();
    if (match('*'))
    {
        type = TOKEN_NLINE_COMMENT;
        skip_multi_line_comment();
    }

    return make_token(type);
}

static void skip_whitespace()
{
    while (is_space())
    {
        switch (next())
        {
        case '\n':
            scan.line++;
            break;
        case '/':
            if (match('*') || match('/'))
                skip_comment();
            break;
        }
        skip();
    }
}
