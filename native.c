#include "native.h"
#include "object_memory.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static element get_file(const char *path)
{

	char  rest[1024] = {0};
	char *tmp        = NULL;
	tmp              = realpath(path, rest);
	FILE *file       = NULL;
	file             = fopen(tmp, "r");

	if (!file)
	{
		fprintf(stderr, "Could not open file \"%s\".\n", path);
		exit(74);
	}

	fseek(file, 0L, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file);

	char *buffer = NULL;
	buffer       = ALLOC(fileSize + 1);

	if (!buffer)
	{
		fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
		exit(74);
	}
	size_t bytesRead  = fread(buffer, sizeof(char), fileSize, file);
	buffer[bytesRead] = '\0';

	value el;
	el.String = buffer;
	el.len    = bytesRead;

	fclose(file);
	return OBJ(el, T_STR);
}

static void append_file(const char *path, const char *data)
{
	FILE *f = NULL;

	char *tmp = NULL;
	tmp       = realpath(path, NULL);
	if (!tmp)
	{
		fprintf(stderr, "Unable to open file: %s\n", tmp);
		exit(1);
	}

	f = fopen(tmp, "a");

	if (!f)
	{
		fprintf(stderr, "Unable to open file: %s\n", path);
		exit(1);
	}

	fprintf(f, "\n%s", data);
	fclose(f);

	tmp = NULL;
	f   = NULL;
}
static void write_file(const char *path, const char *data)
{
	FILE *f = NULL;

	char *tmp = NULL;
	tmp       = realpath(path, NULL);
	if (!tmp)
	{
		fprintf(stderr, "Unable to open file: %s\n", tmp);
		exit(1);
	}

	f = fopen(tmp, "w");

	if (!f)
	{
		fprintf(stderr, "Unable to open file: %s\n", path);
		exit(1);
	}

	fprintf(f, "%s", data);
	fclose(f);

	tmp = NULL;
	f   = NULL;
}

element file_native(int argc, element *argv)
{
	switch (*argv->val.String)
	{
	case 'r':
		return get_file(argv[1].val.String);
	case 'w':
		write_file(argv[1].val.String, argv[2].val.String);
		return Null();
	case 'a':
		append_file(argv[1].val.String, argv[2].val.String);
		return Null();
	default:
		return Null();
	}
}

element square_native(int argc, element *argv)
{
	return _sqr(argv);
}
element clock_native(int argc, element *el)
{
	return Num((double)clock() / CLOCKS_PER_SEC);
}
element string_native(int argc, element *el)
{
	return Null();
}

static void define_native(stack **stk, _key ar, NativeFn n, uint8_t index)
{
	element el            = GEN(_native(n, ar), T_NATIVE);
	*((*stk)->as + index) = el;
}

void define_natives(stack **stk)
{
	define_native(stk, Key("clock", 5), clock_native, 0);
	define_native(stk, Key("square", 6), square_native, 1);
	define_native(stk, Key("file", 4), file_native, 2);
}
