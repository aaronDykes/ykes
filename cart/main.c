#include <stdio.h>
#include "virtual_machine.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

static void repl();
static void run_file(const char *path);
static char *read_file(const char *path);

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

static void repl()
{

    initVM();
    arena ar = arena_alloc(1024 * sizeof(char), ARENA_STR);

    for (;;)
    {
        printf("$ ");

        if (!fgets(ar.as.string, ar.size, stdin))
        {
            printf("\n");
            break;
        }
        if (strcmp(ar.as.string, "end\n") == 0)
            break;

        interpret(ar.as.string);
    }
    arena_free(&ar);
    freeVM();
}

static void run_file(const char *path)
{
    initVM();
    char *source = read_file(path);
    Interpretation result = interpret(source);
    free(source);

    if (result == INTERPRET_COMPILE_ERR)
        exit(65);
    if (result == INTERPRET_RUNTIME_ERR)
        exit(70);

    freeVM();
}

static char *read_file(const char *path)
{

    FILE *file = fopen(path, "rb");

    if (!file)
    {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char *buffer = malloc(fileSize + 1);

    if (!buffer)
    {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;

    /*

            struct stat s;
        int fd = open(path, O_RDONLY);
        int status = fstat(fd, &s);
        int size = s.st_size;

        char *f = (char *)mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
        char *tmp = f;

        munmap(f, size);
        return tmp;
    */
}