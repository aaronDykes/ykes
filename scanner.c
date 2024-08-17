#include "lex_util.h"
#include <stdlib.h>

void init_scanner(const char *src)
{
	scan.start   = src;
	scan.current = src;
	scan.line    = 1;
}
void re_init_scanner(const char *src, int line)
{
	scan.start   = src;
	scan.current = src;
	scan.line    = line;
}

token scan_token(void)
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
	case '?':
		if (match('?'))
			return make_token(TOKEN_CH_NULL_COALESCING);
		return make_token(TOKEN_CH_TERNARY);
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
		if (match('>'))
			return make_token(TOKEN_OP_CAST);
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
	case '`':
		return string(TOKEN_FMT_STR);
	case '"':
		return string(TOKEN_STR);
	case '\\':
		return scan_token();
	}
	return err_token("ERROR: invalid token");
}

static token make_token(int t)
{
	token toke;
	toke.start = scan.start;
	toke.line  = scan.line;
	toke.type  = t;
	toke.size  = (int)(scan.current - scan.start);
	return toke;
}
static token err_token(const char *err)
{
	token toke;
	toke.start = err;
	toke.line  = scan.line;
	toke.type  = TOKEN_ERR;
	toke.size  = strlen(err);
	return toke;
}

static token string(token_t type)
{
	char ch = type == TOKEN_FMT_STR ? '`' : '"';

	while (next() != ch && !end())
	{
		if (match('\\') && check('n'))
		{
			scan.line++;
			if (match('\"'))
				return make_token(type);
		}

		skip();
	}

	if (end())
		return err_token("Unterminated string");

	skip();
	return make_token(type);
}
static token number(void)
{

	while (digit(next()))
		skip();
	if (next() == '.' && digit(peek(1)))
	{
		skip();
		while (digit(next()))
			skip();
	}

	return make_token(TOKEN_NUMBER);
}
static token id(void)
{
	while (digit(next()) || alpha(next()))
		skip();

	return make_token(id_type());
}
static token character(void)
{
	nskip(2);
	return make_token(TOKEN_CHAR);
}

static token strict_toke(int t)
{
	nskip(2);
	return make_token(t);
}

static int id_type(void)
{
	switch (*scan.start)
	{
	case 'A':
		return check_keyword(1, 4, "rray", TOKEN_ALLOC_ARRAY);
	case 'a':
		if (scan.current - scan.start > 1)
		{
			switch (scan.start[1])
			{
			case 'r':
				return check_keyword(2, 3, "ray", TOKEN_TYPE_ARRAY);
			case 'n':
				return check_keyword(2, 1, "d", TOKEN_OP_AND);
			}
		}
	case 'b':
		switch (scan.start[1])
		{
		case 'r':
			return check_keyword(2, 3, "eak", TOKEN_BREAK);
		case 'o':
			return check_keyword(2, 2, "ol", TOKEN_STORAGE_TYPE_BOOL);
		}
	case 'c':
		switch (scan.start[1])
		{
		case 'a':
			return check_keyword(2, 2, "se", TOKEN_CASE);
		case 'l':
			switch (scan.start[2])
			{
			case 'o':
				return check_keyword(3, 2, "ck", TOKEN_CLOCK);
			case 'a':
				return check_keyword(3, 2, "ss", TOKEN_CLASS);
			}
		case 'h':
			return check_keyword(2, 2, "ar", TOKEN_STORAGE_TYPE_CHAR);
		}
	case 'd':
		return check_keyword(1, 6, "efault", TOKEN_DEFAULT);
	case 'D':
		return check_keyword(1, 6, "oubles", TOKEN_ALLOC_DOUBLES);
	case 'e':
		switch (scan.start[1])
		{
		case 'a':
			return check_keyword(2, 2, "ch", TOKEN_EACH);
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
			case 'i':
				return check_keyword(2, 2, "le", TOKEN_FILE);
			}
	case 'i':
		switch (scan.start[1])
		{
		case 'm':
			return check_keyword(2, 4, "port", TOKEN_INCLUDE);
		}
		return check_keyword(1, 1, "f", TOKEN_IF);
	case 'n':
		switch (scan.start[1])
		{
		case 'u':
			switch (scan.start[2])
			{
			case 'l':
				return check_keyword(3, 1, "l", TOKEN_NULL);
			}
			return check_keyword(2, 1, "m", TOKEN_STORAGE_TYPE_NUM);
		}

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
				if (scan.current - scan.start > 2)
				{
					switch (scan.start[2])
					{
					case 't':
						return check_keyword(
						    3, 3, "urn", TOKEN_RETURN
						);
					case 'a':
						return check_keyword(3, 1, "d", TOKEN_READ);
					}
				}
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
			case 't':
				switch (scan.start[2])
				{
				case 'r':
					return check_keyword(
					    2, 4, "ring", TOKEN_STORAGE_TYPE_STR
					);
				case 'a':
					return check_keyword(
					    2, 3, "ack", TOKEN_TYPE_STACK
					);
				}
			}

		return check_keyword(1, 1, "r", TOKEN_FUNC);
	case 'S':
		if (scan.current - scan.start > 1)
			switch (scan.start[1])
			{
			case 't':
				switch (scan.start[2])
				{
				case 'a':
					return check_keyword(
					    3, 2, "ck", TOKEN_ALLOC_STACK
					);
				}
			}
	case 't':
		if (scan.current - scan.start > 1)
			switch (scan.start[1])
			{
			case 'a':
				return check_keyword(2, 3, "ble", TOKEN_TYPE_TABLE);
			case 'h':
				return check_keyword(2, 2, "is", TOKEN_THIS);
			case 'r':
				return check_keyword(2, 2, "ue", TOKEN_TRUE);
			}
	case 'T':
		return check_keyword(1, 4, "able", TOKEN_TABLE);

	case 'v':
		if (scan.current - scan.start > 1)
		{
			switch (scan.start[1])
			{
			case 'a':
				return check_keyword(2, 1, "r", TOKEN_VAR);
			case 'e':
				return check_keyword(2, 4, "ctor", TOKEN_TYPE_VECTOR);
			}
		}
	case 'V':
		return check_keyword(1, 5, "ector", TOKEN_ALLOC_VECTOR);
	case 'w':
		return check_keyword(1, 4, "hile", TOKEN_WHILE);
	}
	return TOKEN_ID;
}
static int check_keyword(int start, int end, const char *str, int t)
{

	if (((int)(scan.current - scan.start) == start + end) &&
	    memcmp(scan.start + start, str, end) == 0)
		return t;

	return TOKEN_ID;
}

static char next(void)
{
	return *scan.current;
}
static char advance(void)
{
	return *scan.current++;
}
static char peek(int n)
{
	return scan.current[n];
}

static void skip(void)
{
	scan.current++;
}
static void nskip(int n)
{
	for (int i = 0; i < n; i++)
		skip();
}

static bool is_space(void)
{
	return next() == ' ' || next() == '\t' || next() == '\\' ||
	       next() == '\r' || next() == '\n';
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
static bool end(void)
{
	return *scan.current == '\0';
}
static bool digit(char c)
{
	return (c >= '0' && c <= '9');
}
static bool alpha(char c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
}

static void skip_line_comment(void)
{
	for (; *scan.current && *scan.current != '\n'; skip())
		;

	if (!end())
		scan.line++;

	skip();
}
static void skip_multi_line_comment(void)
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
static token skip_comment(void)
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

static void skip_whitespace(void)
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
