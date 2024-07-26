#include "virtual_machine.h"
#include "compiler.h"
#include "error.h"
#include "vm_util.h"
#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

void initVM(void)
{

    initialize_global_memory();

    machine.stack.main = NULL;
    machine.stack.obj = NULL;
    machine.glob = NULL;

    machine.stack.main = GROW_STACK(NULL, MIN_SIZE);
    machine.stack.obj = GROW_STACK(NULL, MIN_SIZE);
    machine.glob = GROW_TABLE(NULL, MIN_SIZE);

    machine.current_instance = NULL;
    machine.init_fields = NULL;

    machine.count.argc = 0;
    machine.count.frame = 0;
    machine.count.cargc = 0;

    define_native(Key("clock", 5), clock_native);
    define_native(Key("square", 6), square_native);
    define_native(Key("file", 4), file_native);
}
void freeVM(void)
{
    FREE_TABLE(machine.glob);
    FREE_STACK(machine.stack.main);
    FREE_STACK(machine.stack.obj);

    machine.glob = NULL;
    machine.stack.main = NULL;
    machine.stack.obj = NULL;

    destroy_global_memory();
}

static void reset_vm_stack(void)
{
    reset_stack(machine.stack.main);
    machine.count.frame = 0;
}

static void runtime_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = machine.count.frame - 1; i >= 0; i--)
    {

        CallFrame *frame = &machine.frames[i];
        function *func = frame->closure->func;
        int line = frame->closure->func->ch.lines.bytes[i];

        if (!func->name.val)
            fprintf(stderr, "script\n");
        else
            fprintf(stderr, "%s()\n", func->name.val);
        fprintf(stderr, "[line %d] in script\n", line);
    }

    reset_vm_stack();
}

static void define_native(_key ar, NativeFn n)
{
    element el = GEN(_native(n, ar), T_NATIVE);
    push(&machine.stack.obj, el);
}

static inline element clock_native(int argc, element *el)
{
    return Num((double)clock() / CLOCKS_PER_SEC);
}
static element get_file(const char *path)
{

    char rest[PATH_MAX] = {0};
    char *tmp = NULL;
    tmp = realpath(path, rest);
    FILE *file = NULL;
    file = fopen(tmp, "r");

    if (!file)
    {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char *buffer = NULL;
    buffer = ALLOC(fileSize + 1);

    if (!buffer)
    {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    buffer[bytesRead] = '\0';

    value el;
    el.String = buffer;
    el.len = bytesRead;

    fclose(file);
    return OBJ(el, T_STR);
}

static void append_file(const char *path, const char *data)
{
    FILE *f = NULL;

    char *tmp = NULL;
    tmp = realpath(path, NULL);
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
    f = NULL;
}
static void write_file(const char *path, const char *data)
{
    FILE *f = NULL;

    char *tmp = NULL;
    tmp = realpath(path, NULL);
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
    f = NULL;
}

static inline element file_native(int argc, element *argv)
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

static inline element square_native(int argc, element *argv)
{
    return _sqr(*argv);
}

static bool call(closure *c, uint8_t argc)
{

    if (c->func->arity != argc)
    {
        runtime_error("ERROR: Expected `%d` args, but got `%d`.", c->func->arity, argc);
        return false;
    }

    if (machine.count.frame == FRAMES_MAX)
    {
        runtime_error("ERROR: stack overflow.");
        return false;
    }

    CallFrame *frame = &machine.frames[machine.count.frame++];
    frame->closure = c;
    frame->closure->upvals = c->upvals;
    frame->ip = c->func->ch.ip.bytes;
    frame->ip_start = c->func->ch.ip.bytes;
    frame->slots = machine.stack.main->top - argc - 1;
    return true;
}

Interpretation interpret(const char *src)
{

    function *func = NULL;

    if (!(func = compile(src)))
        return INTERPRET_RUNTIME_ERR;

    closure *clos = _closure(func);
    call(clos, 0);
    machine.frames[machine.count.frame - 1].slots = machine.stack.main->top;

    push(&machine.stack.main, GEN(clos, T_CLOSURE));

    close_upvalues(machine.stack.main->top - 1);
    return run();
}
Interpretation interpret_path(const char *src, const char *path, const char *name)
{

    function *func = NULL;

    if (!(func = compile_path(src, path, name)))
        return INTERPRET_RUNTIME_ERR;

    closure *clos = _closure(func);
    call(clos, 0);
    machine.frames[machine.count.frame - 1].slots = machine.stack.main->top;

    push(&machine.stack.main, GEN(clos, T_CLOSURE));

    close_upvalues(machine.stack.main->top - 1);
    return run();
}

static bool call_value(element el, uint8_t argc)
{
    switch (el.type)
    {

    case T_CLOSURE:
        return call(CLOSURE(el), argc);
    case T_NATIVE:
    {
        element res = NATIVE(el)->fn(argc, machine.stack.main->top - argc);
        machine.stack.main->count -= (argc + 1);
        machine.stack.main->top -= (argc + 1);
        push(&machine.stack.main, res);
        return true;
    }
    default:
        break;
    }
    runtime_error("ERROR: Can only call functions and classes.");
    return false;
}

static upval *capture_upvalue(element *s)
{
    upval *prev = NULL;
    upval *curr = machine.open_upvals;

    for (; curr && curr->index && curr->index < s; curr = curr->next)
        prev = curr;

    if (curr && curr->index == s)
        return curr;

    upval *new = _upval(s);
    new->next = curr;

    if (prev)
        prev->next = new;
    else
        machine.open_upvals = new;
    return new;
}

static void close_upvalues(element *local)
{
    while (machine.open_upvals && machine.open_upvals->index >= local)
    {
        upval *up = machine.open_upvals;
        up->closed = *machine.open_upvals->index;
        up->index = &up->closed;
        machine.open_upvals = up->next;
    }
}

static bool not_null(element el)
{
    switch (el.type)
    {
    case T_STR:
        return el.val.String ? true : false;
    case T_TABLE:
    case T_CLOSURE:
    case T_CLASS:
    case T_INSTANCE:
    case T_STACK:
        return el.obj ? true : false;
    default:
        return false;
    }
}
static bool null(element el)
{
    switch (el.type)
    {
    case T_STR:
        return el.val.String ? false : true;
    case T_TABLE:
    case T_CLOSURE:
    case T_CLASS:
    case T_INSTANCE:
    case T_STACK:
        return el.obj ? false : true;
    default:
        return true;
    }
}

Interpretation run(void)
{

    CallFrame *frame = &machine.frames[machine.count.frame - 1];

#define READ_BYTE() \
    (*frame->ip++)
#define READ_SHORT() \
    (((READ_BYTE() << 8) & 0xFF) | READ_BYTE() & 0xFF)
#define READ_CONSTANT() \
    (*(frame->closure->func->ch.constants->as + READ_BYTE()))

#define POP() \
    (pop(&machine.stack.main))
#define POPN(n) \
    (popn(&machine.stack.main, n))
#define PUSH(ar) \
    (push(&machine.stack.main, ar))
#define PEEK() \
    (*(machine.stack.main->top - 1))
#define NPEEK(N) \
    (*(machine.stack.main->top + (-1 - N)))

#define FALSEY() \
    (!PEEK().val.Bool)
#define TRUTHY() \
    (PEEK().val.Bool)

#define LOCAL() \
    (*(frame->slots + READ_BYTE()))
#define NLOCAL(n) \
    (*(frame->slots + n))

#define OBJECT() \
    (*(frame->closure->func->ch.constants->as + READ_BYTE()))

#define GET(ar) \
    (find_entry(&machine.glob, ar))
#define SET(a, b) \
    (write_table(machine.glob, a, b))

    for (;;)
    {
#ifdef DEBUG_TRACE_EXECUTION
        for (element *v = machine.stack.main->as; v < machine.stack.main->top; v++)
            print(*v);
        disassemble_instruction(&frame->closure->func->ch,
                                (int)(frame->ip - frame->ip_start));
#endif

        switch (READ_BYTE())
        {
        case OP_CONSTANT:
            PUSH(READ_CONSTANT());
            break;
        case OP_CLOSURE:
        {
            element e = READ_CONSTANT();
            OBJECT() = e;

            for (int i = 0; i < CLOSURE(e)->uargc; i++)
                CLOSURE(e)->upvals[i] =
                    (READ_BYTE())
                        ? capture_upvalue(frame->slots + READ_BYTE())
                        : frame->closure->upvals[READ_BYTE()];
        }
        break;
        case OP_METHOD:
        {
            element e = READ_CONSTANT();

            for (int i = 0; i < CLOSURE(e)->uargc; i++)
                CLOSURE(e)->upvals[i] =
                    (READ_BYTE())
                        ? capture_upvalue(frame->slots + READ_BYTE())
                        : frame->closure->upvals[READ_BYTE()];
        }
        break;

        case OP_GET_UPVALUE:
            PUSH((*frame->closure->upvals + READ_BYTE())->closed);
            break;
        case OP_SET_UPVALUE:
            ((*frame->closure->upvals + READ_BYTE()))->closed = *(machine.stack.main->top - 1);
            break;

        case OP_NEG:
            *(--machine.stack.main->top) = _neg(*(machine.stack.main->top++));
            break;

        case OP_INC_GLO:
        {
            element key = READ_CONSTANT();
            element ar = _inc(GET(key.key));
            SET(key.key, ar);
            PUSH(ar);
            break;
        }
        case OP_DEC_GLO:
        {
            element key = READ_CONSTANT();
            element ar = _dec(GET(key.key));
            SET(key.key, ar);
            PUSH(ar);
            break;
        }
        case OP_INC_LOC:
        {
            uint8_t index = READ_BYTE();
            element el = _inc(NLOCAL(index));
            NLOCAL(index) = el;
            PUSH(el);
            break;
        }
        case OP_DEC_LOC:
        {
            uint8_t index = READ_BYTE();
            element el = _dec(NLOCAL(index));
            NLOCAL(index) = el;
            PUSH(el);
            break;
        }
        case OP_INC:
            *--machine.stack.main->top = _inc(*machine.stack.main->top++);
            break;
        case OP_DEC:
            *--machine.stack.main->top = _dec(*machine.stack.main->top++);
            break;
        case OP_POPN:
            POPN(READ_CONSTANT().val.Num);
            break;
        case OP_POP:
            POP();
            break;
        case OP_ADD:
            PUSH(_add(POP(), POP()));
            break;
        case OP_SUB:
            PUSH(_sub(POP(), POP()));
            break;
        case OP_MUL:
            PUSH(_mul(POP(), POP()));
            break;
        case OP_MOD:
            PUSH(_mod(POP(), POP()));
            break;
        case OP_DIV:
            PUSH(_div(POP(), POP()));
            break;
        case OP_EQ:
            PUSH(_eq(POP(), POP()));
            break;
        case OP_NE:
            PUSH(_ne(POP(), POP()));
            break;

        case OP_LT:
            PUSH(_lt(POP(), POP()));
            break;
        case OP_LE:
            PUSH(_le(POP(), POP()));
            break;
        case OP_GT:
            PUSH(_gt(POP(), POP()));
            break;
        case OP_GE:
            PUSH(_ge(POP(), POP()));
            break;
        case OP_OR:
            PUSH(_or(POP(), POP()));
            break;
        case OP_AND:
            PUSH(_and(POP(), POP()));
            break;

        case OP_RESET_ARGC:
            machine.count.cargc = 0;
            machine.count.argc = 0;
            break;

        case OP_NULL:
            PUSH(Null());
            break;
        case OP_JMPF:
            frame->ip += (READ_SHORT() * FALSEY());
            break;
        case OP_JMPL:
            frame->ip = frame->ip_start + *(frame->closure->func->ch.cases.bytes + READ_BYTE());
            break;
        case OP_SET_PROP:
        {
            element el = PEEK();
            // element inst = NPEEK(1);
            if (!machine.current_instance && !machine.init_fields)
            {
                runtime_error("ERROR: Can only set properties of an instance.");
                return INTERPRET_RUNTIME_ERR;
            }

            if (machine.init_fields)
                write_table(machine.init_fields, READ_CONSTANT().key, el);
            else
                write_table(machine.current_instance->fields, READ_CONSTANT().key, el);
            POP();
            break;
        }

        case OP_GET_PROP:
        {
            if (!machine.current_instance && !machine.init_fields)
            {
                runtime_error("ERROR: Only instances contain properties.");
                return INTERPRET_RUNTIME_ERR;
            }

            _key name = READ_CONSTANT().key;

            element n = (machine.init_fields)
                            ? find_entry(&machine.init_fields, name)
                            : find_entry(&machine.current_instance->fields, name);

            if (n.type != T_NULL)
            {
                PUSH(n);
                break;
            }

            runtime_error("ERROR: Undefined property '%s'.", name.val);
            return INTERPRET_RUNTIME_ERR;
        }

        case OP_CALL:
        {
            uint8_t argc = READ_BYTE();
            if (!call_value(NPEEK(argc), argc))
                return INTERPRET_RUNTIME_ERR;

            frame = (machine.frames + (machine.count.frame - 1));
            machine.count.argc = (argc == 0) ? 1 : argc;
            machine.count.cargc = 1;
            machine.count.slot = machine.stack.main->count;

            break;
        }
        case OP_JMPT:
            frame->ip += (READ_SHORT() * TRUTHY());
            break;
        case OP_JMP_NIL:
        {
            uint16_t offset = READ_SHORT();
            if (null(PEEK()))
                frame->ip += offset;
            break;
        }
        case OP_JMP_NOT_NIL:
        {
            uint16_t offset = READ_SHORT();
            if (not_null(PEEK()))
                frame->ip += offset;
            break;
        }
        break;
        case OP_JMP:
            frame->ip += READ_SHORT();
            break;
        case OP_LOOP:
            frame->ip -= READ_SHORT();
            break;
        case OP_GET_LOCAL:
            PUSH(LOCAL());
            break;
        case OP_SET_LOCAL:
            LOCAL() = PEEK();
            break;
        case OP_SET_LOCAL_PARAM:
            LOCAL() = (machine.count.cargc < machine.count.argc)
                          ? *(frame->slots + machine.count.cargc++)
                          : PEEK();
            break;
        case OP_GET_OBJ:
            PUSH(OBJECT());
            break;
        case OP_SET_OBJ:
            OBJECT() = READ_CONSTANT();
            break;
        case OP_CLASS:
        {
            class *c = CLASS(OBJECT());
            machine.init_fields = GROW_TABLE(c->closures, (c->closures - 1)->len);
            PUSH(GEN(c->init, T_CLOSURE));
            break;
        }
        case OP_ALLOC_INSTANCE:
            machine.current_instance = _instance(CLASS(OBJECT()));
            machine.current_instance->fields =
                (machine.init_fields)
                    ? machine.init_fields
                    : GROW_TABLE(machine.current_instance->classc->closures, (machine.current_instance->classc->closures - 1)->len);
            machine.init_fields = NULL;
            PUSH(GEN(machine.current_instance, T_INSTANCE));
            break;
        case OP_RM:
            FREE_OBJ(POP());
            break;
        case OP_ALLOC_TABLE:
            if (PEEK().type != T_NUM)
            {
                runtime_error("ERROR: table argument must be a numeric value.");
                return INTERPRET_RUNTIME_ERR;
            }
            PUSH(GEN(GROW_TABLE(NULL, POP().val.Num), T_TABLE));
            break;
        case OP_GET_GLOBAL:
        {

            _key var = READ_CONSTANT().key;
            element el = GET(var);

            if (el.type != T_NULL)
            {
                if (el.type == T_INSTANCE)
                    machine.current_instance = INSTANCE(el);
                PUSH(el);
                break;
            }

            runtime_error("ERROR: Undefined property '%s'.", var.val);
            return INTERPRET_RUNTIME_ERR;
        }
        case OP_GLOBAL_DEF:
        {
            element el = READ_CONSTANT();
            element res = POP();

            if (GET(el.key).type == T_NULL)
            {
                error("Duplicate global variable identifier: %s\n", el.key.val);
                return INTERPRET_RUNTIME_ERR;
            }

            if (res.type == T_CLOSURE)
                CLOSURE(res)->func->name = el.key;
            SET(el.key, res);
            break;
        }
        case OP_SET_GLOBAL:
        {
            element el = READ_CONSTANT();
            element res = POP();

            if (res.type == T_CLOSURE)
                CLOSURE(res)->func->name = el.key;
            SET(el.key, res);
            break;
        }
        case OP_SET_FUNC_VAR:
        {
            element el = READ_CONSTANT();
            element res = (machine.count.cargc < machine.count.argc)
                              ? *(frame->slots + machine.count.cargc++)
                              : POP();

            if (res.type == T_CLOSURE)
                CLOSURE(res)->func->name = el.key;
            SET(el.key, res);
            break;
        }

        case OP_PRINT:
            print(POP());
            break;
        case OP_RETURN:
        {
            element el = POP();
            --machine.count.frame;

            if (machine.count.frame == 0)
                return INTERPRET_SUCCESS;

            machine.stack.main->count -= (machine.stack.main->count - machine.count.slot);
            machine.stack.main->top = frame->slots;
            PUSH(el);

            frame = &machine.frames[machine.count.frame - 1];
            break;
        }
        }
    }

#undef RM
#undef POP
#undef WRITE_GLOB
#undef WRITE_PARAM
#undef FIND_GLOB
#undef FIND_PARAM
#undef PPUSH
#undef CPUSH
#undef PUSH
#undef JUMP
#undef LOCAL
#undef POPN
#undef POP
#undef FALSEY
#undef NPEEK
#undef PEEK
#undef GET_NATIVE
#undef GET_FUNC
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_BYTE
}
