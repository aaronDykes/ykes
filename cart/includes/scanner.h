#ifndef _SCANNER_H
#define _SCANNER_H
#include "common.h"
#include "token_type.h"
#include <stdio.h>
#include <string.h>

struct token
{
	token_t     type;
	int         line;
	int         size;
	const char *start;
};

struct scanner
{
	int         line;
	const char *start;
	const char *current;
};

typedef struct token token;
typedef token       *Token;

typedef struct scanner scanner;
static scanner         scan;

void  init_scanner(const char *src);
token scan_token(void);

#endif
