#ifndef _LEX_UTIL_H
#define _LEX_UTIL_H

static int id_type();
static int check_keyword(int start, int end, const char *str, int t);

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
static void skip_whitespace();

#endif