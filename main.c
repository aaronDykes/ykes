#include "object_string.h"
#include "virtual_machine.h"
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static void  repl(void);
static void  run_file(const char *path);
static char *read_file(char *path);

int main(int argc, char **argv)
{

	if (argc == 1)
		repl();
	else if (argc == 2)
		run_file(argv[1]);
	else
	{
		fprintf(stderr, "USAGE: ykes [path]\n");
		exit(69);
	}

	return EXIT_SUCCESS;
}

static void repl(void)
{

	initVM();
	buffer b;
	// arena ar = GROW_ARENA(NULL, 1024 * sizeof(char), ARENA_STR);

	init_natives();
	for (;;)
	{
		printf("$ ");

		char ch = 0;
		b       = _buffer(INIT_SIZE);

		while ((ch = getchar()) != '\n')
			write_buffer(&b, ch);

		write_buffer(&b, '\0');

		if (strcmp(b.bytes, "end") == 0)
			break;
		if (b.count == 2 && *b.bytes == 'q')
			break;

		interpret(b.bytes);
		b.count = 0;
	}
	free_buffer(&b);
	freeVM();
}

static int str_len(char *str)
{
	int   count = 0;
	char *s     = str;

	while ((*s++))
		count++;
	return count;
}

static void strip_path(char *str)
{
	int   len = str_len(str) - 1;
	char *s   = str + len;

	for (; *s != '/'; --s, --len)
		;

	str[len + 1] = '\0';
}

static char *get_name(char *path)
{
	int   len = strlen(path) - 1;
	char *tmp = path + len;

	int count;
	for (count = 0; tmp[-1] != '/'; --tmp, count++)
		;

	char *file = ALLOC((count + 1) * sizeof(char));

	strcpy(file, tmp);

	return file;
}

static char *get_full_path(char *path)
{
	char resolved[PATH_MAX] = {0};

	char *ptr = NULL;

	if (realpath(path, resolved) != NULL)
	{
		ptr = ALLOC(strlen(resolved) + 1);
		strcpy(ptr, resolved);
	}
	else
	{
		fprintf(stderr, "Could not open file \"%s\".\n", path);
		exit(74);
	}
	return ptr;
}

static void run_file(const char *path)
{
	initVM();

	char *source = NULL;
	source       = read_file(get_full_path((char *)path));

	char *name = NULL;
	name       = get_name((char *)path);
	strip_path((char *)path);

	Interpretation result = interpret_path(source, path, name);
	FREE(source);

	if (result == INTERPRET_COMPILE_ERR)
		exit(65);
	if (result == INTERPRET_RUNTIME_ERR)
		exit(70);

	freeVM();
}

static char *read_file(char *path)
{

	FILE *file = NULL;
	if (path)
		file = fopen(path, "rb");
	else if (!file)
	{
		fprintf(stderr, "Could not open file \"%s\".\n", path);
		exit(74);
	}

	fseek(file, 0L, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file);

	char *buffer = ALLOC(fileSize + 1);

	if (!buffer)
	{
		fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
		exit(74);
	}
	size_t bytesRead  = fread(buffer, sizeof(char), fileSize, file);
	buffer[bytesRead] = '\0';

	fclose(file);
	FREE(path);
	return buffer;

	/*

	        struct stat s;
	    int fd = open(path, O_RDONLY);
	    int status = fstat(fd, &s);
	    int size = s.st_size;

	    void *f = (char *)mmap(0, PAGE, PROT_READ, MAP_PRIVATE, -1, 0);
	    char *tmp = f;

	    munmap(f, size);
	    return tmp;
	*/
}
