#include "scanner.h"
#include <stdlib.h>
#include "lex_util.h"

static token make_token(int t);
static token err_token(const char *err);
static token string();
static token number();
static token id();
static token character();
static token skip_comment();
static token strict_toke(int t);

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
    case '[':
        return make_token(TOKEN_CH_LSQUARE);
    case ']':
        return make_token(TOKEN_CH_RSQUARE);
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
        if (match('='))
            return make_token(TOKEN_DIV_ASSIGN);
        if (check('/') || check('*'))
            return skip_comment();
        return make_token(TOKEN_OP_DIV);
    case '*':
        if (match('='))
            return make_token(TOKEN_MUL_ASSIGN);
        return make_token(TOKEN_OP_MUL);
    case '-':
        if (match('='))
            return make_token(TOKEN_SUB_ASSIGN);
        return make_token(match('-') ? TOKEN_OP_DEC : TOKEN_OP_SUB);
    case '+':
        if (match('='))
            return make_token(TOKEN_ADD_ASSIGN);
        return make_token(match('+') ? TOKEN_OP_INC : TOKEN_OP_ADD);
    case '%':
        if (match('='))
            return make_token(TOKEN_MOD_ASSIGN);
        return make_token(TOKEN_OP_MOD);
    case '&':
        if (match('='))
            return make_token(TOKEN_AND_ASSIGN);
        return make_token(match('&') ? TOKEN_SC_AND : TOKEN_LG_AND);
    case '|':
        if (match('='))
            return make_token(TOKEN_OR__ASSIGN);
        return make_token(match('|') ? TOKEN_SC_OR : TOKEN_LG_OR);
    case '!':
        if (check('=') && check_peek(1, '='))
            return strict_toke(TOKEN_OP_SNE);
        return make_token(match('=') ? TOKEN_OP_NE : TOKEN_OP_BANG);
    case '=':
        if (check('=') && check_peek(1, '='))
            return strict_toke(TOKEN_OP_SEQ);
        return make_token(match('=') ? TOKEN_OP_EQ : TOKEN_OP_ASSIGN);
    case '>':
        return make_token(match('=') ? TOKEN_OP_GE : TOKEN_OP_GT);
    case '<':
        return make_token(match('=') ? TOKEN_OP_LE : TOKEN_OP_LT);
    case '\'':
        return character();
    case '"':
        return string();
    }
    return err_token("ERROR: invalid token");
}

static token make_token(int t)
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
    nskip(2);
    return make_token(TOKEN_CHAR);
}

static token strict_toke(int t)
{
    nskip(2);
    return make_token(t);
}

static int id_type()
{
    switch (*scan.start)
    {
    case 'a':
        return check_keyword(1, 2, "nd", TOKEN_OP_AND);
    case 'b':
        if (scan.current - scan.start > 1)
            switch (scan.start[1])
            {
            case 'y':
                return check_keyword(2, 3, "te", TOKEN_BTYE);
            case 'r':
                return check_keyword(2, 3, "eak", TOKEN_BREAK);
            }
    case 'B':
        return check_keyword(1, 4, "ools", TOKEN_ALLOC_BOOLS);
    case 'c':

        if (scan.current - scan.start > 1)
            switch (scan.start[1])
            {
            case 'a':
                return check_keyword(2, 2, "se", TOKEN_CASE);
            case 'l':

                if (scan.current - scan.start > 1)
                    switch (scan.start[2])
                    {
                    case 'o':
                        return check_keyword(3, 2, "ck", TOKEN_CLOCK);
                    case 's':
                        return check_keyword(3, 2, "ss", TOKEN_CLASS);
                    }
            }
    case 'd':
        return check_keyword(1, 6, "efault", TOKEN_DEFAULT);
    case 'D':
        return check_keyword(1, 6, "oubles", TOKEN_ALLOC_DOUBLES);
    case 'e':
        if (scan.current - scan.start > 1)
            switch (scan.start[1])
            {
            case 'l':
                switch (scan.start[2])
                {
                case 's':
                    return check_keyword(3, 1, "e", TOKEN_ELSE);
                case 'i':
                    return check_keyword(3, 1, "f", TOKEN_ELIF);
                }
            }
    case 'f':
        if (scan.current - scan.start > 1)
            switch (scan.start[1])
            {
            case 'o':
                return check_keyword(2, 1, "r", TOKEN_FOR);
            case 'a':
                return check_keyword(2, 3, "lse", TOKEN_FALSE);
            case 'r':
                return check_keyword(2, 2, "ee", TOKEN_OP_FREE);
            }
    case 'i':
        return check_keyword(1, 1, "f", TOKEN_IF);
    case 'I':
        return check_keyword(1, 3, "nts", TOKEN_ALLOC_INTS);
    case 'l':
        if (scan.current - scan.start > 1)
            switch (scan.start[1])
            {
            case 'l':
                return check_keyword(2, 3, "int", TOKEN_LLINT);
            case 'i':
                return check_keyword(2, 2, "nt", TOKEN_LINT);
            }
    case 'L':
        return check_keyword(1, 4, "ongs", TOKEN_ALLOC_LONGS);
    case 'n':
        return check_keyword(1, 3, "ull", TOKEN_NULL);
    case 'o':
        return check_keyword(1, 1, "r", TOKEN_OP_OR);
    case 'p':
        if (scan.current - scan.start > 1)
            switch (scan.start[1])
            {
            case 'r':
                return check_keyword(2, 3, "ime", TOKEN_PRIME);
            case 'o':
                return check_keyword(2, 2, "ut", TOKEN_PRINT);
            }
    case 'r':
        if (scan.current - scan.start > 1)
            switch (scan.start[1])
            {
            case 'e':
                return check_keyword(2, 4, "turn", TOKEN_RETURN);
            }
        return check_keyword(1, 1, "m", TOKEN_OP_REM);
    case 's':
        if (scan.current - scan.start > 1)
            switch (scan.start[1])
            {
            case 'w':
                return check_keyword(2, 4, "itch", TOKEN_SWITCH);
            case 'u':
                return check_keyword(2, 3, "per", TOKEN_SUPER);
            case 'q':
                return check_keyword(2, 4, "uare", TOKEN_SQRT);
            }

        return check_keyword(1, 1, "r", TOKEN_FUNC);
    case 'S':
        return check_keyword(1, 5, "tring", TOKEN_ALLOC_STR);
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
static int check_keyword(int start, int end, const char *str, int t)
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

static bool check_peek(int n, char expected)
{
    return peek(n) == expected;
}
static bool check(char expected)
{
    return next() == expected;
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
    for (; *scan.current && *scan.current != '\n'; skip())
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
    if (scan.current[1] == '/' || next() == '/')
        skip_line_comment();
    else if (scan.current[1] == '*' || next() == '*')
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
            if (scan.current[1] == '*' || scan.current[1] == '/')
                skip_comment();
            break;
        }
        skip();
    }
}
