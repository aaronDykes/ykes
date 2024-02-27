#include "compiler.h"
#include "arena_math.h"
#include <stdarg.h>

static void check_stack_size();
static void reset_stack();

void initVM()
{

    initialize_global_memory(PAGE);
    machine.max_size = STACK_SIZE;
    machine.current_size = 0;
    machine.stack = GROW_ARENA(NULL, (size_t)STACK_SIZE);
    init_dict(&machine.d);
    init_dict(&machine.glob);
    reset_stack();
}
void freeVM()
{
    free_dict(&machine.d);
    free_dict(&machine.glob);
    FREE_ARENA(machine.stack);
    machine.stack_top = NULL;
    destroy_global_memory();
}

static void reset_stack()
{
    machine.stack_top = machine.stack;
}
static void runtime_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    fprintf(stderr, "[line %d] in script\n", machine.ch->line);
    reset_stack();
}

static void check_stack_size()
{
    if (machine.current_size + 1 >= machine.max_size)
    {
        machine.max_size *= INC;
        machine.stack = GROW_ARENA(machine.stack, machine.max_size);
        reset_stack();
    }
}

void push(arena ar)
{
    check_stack_size();
    machine.current_size++;
    *machine.stack_top++ = ar;
}

Interpretation interpret(const char *src)
{
    chunk ch;
    init_chunk(&ch);

    if (!compile(src, &ch))
    {
        free_chunk(&ch);
        return INTERPRET_COMPILE_ERR;
    }

    machine.ch = &ch;
    machine.ip = ch.op_codes;
    Interpretation res = run();

    free_chunk(&ch);
    return res;
}

static arena _neg(arena n)
{
    arena ar = n;

    switch (ar.type)
    {
    case ARENA_DOUBLE:
        ar.as.Double = -ar.as.Double;
        break;
    case ARENA_INT:
        ar.as.Int = -ar.as.Int;
        break;
    case ARENA_LONG:
        ar.as.Long = -ar.as.Long;
        break;
    case ARENA_BOOL:
        ar.as.Bool = !ar.as.Bool;
        break;
    case ARENA_NULL:
        return Bool(true);
    default:
        runtime_error("ERROR: negation type mismatch");
    }
    return ar;
}
static arena _add(arena a, arena b)
{

    // if (a.type == ARENA_VAR)
    //     a = find_entry(&machine.glob.map, &a);
    // if (b.type == ARENA_VAR)
    //     b = find_entry(&machine.glob.map, &b);

    switch (b.type)
    {
    case ARENA_CHAR:
        return add_arena_char(b.as.Char, a);
    case ARENA_DOUBLE:
        return add_arena_double(b.as.Double, a);
    case ARENA_INT:
        return add_arena_int(b.as.Int, a);
    case ARENA_LONG:
        return add_arena_long(b.as.Long, a);
    case ARENA_STR:
        return append(b, a);
    }
    return a;
}

static arena _sub(arena a, arena b)
{

    // if (a.type == ARENA_VAR)
    //     a = find_entry(&machine.glob.map, &a);
    // if (b.type == ARENA_VAR)
    //     b = find_entry(&machine.glob.map, &b);

    switch (b.type)
    {
    case ARENA_DOUBLE:
        return sub_arena_double(b.as.Double, a);
    case ARENA_INT:
        return sub_arena_int(b.as.Int, a);
    case ARENA_CHAR:
        return sub_arena_char(b.as.Char, a);
    case ARENA_LONG:
        return sub_arena_long(b.as.Long, a);
    }
    return b;
}

static arena _mul(arena a, arena b)
{

    // if (a.type == ARENA_VAR)
    //     a = find_entry(&machine.glob.map, &a);
    // if (b.type == ARENA_VAR)
    //     b = find_entry(&machine.glob.map, &b);

    switch (a.type)
    {
    case ARENA_DOUBLE:
        return mul_arena_double(a.as.Double, b);
    case ARENA_CHAR:
        return mul_arena_char(a.as.Char, b);
    case ARENA_INT:
        return mul_arena_int(a.as.Int, b);
    case ARENA_LONG:
        return add_arena_long(a.as.Long, b);
    }
    return a;
}

static arena _div(arena a, arena b)
{

    // if (a.type == ARENA_VAR)
    //     a = find_entry(&machine.glob.map, &a);
    // if (b.type == ARENA_VAR)
    //     b = find_entry(&machine.glob.map, &b);

    switch (b.type)
    {
    case ARENA_CHAR:
        return div_arena_char(b.as.Char, a);
    case ARENA_DOUBLE:
        return div_arena_double(b.as.Double, a);
    case ARENA_INT:
        return div_arena_int(b.as.Int, a);
    case ARENA_LONG:
        return add_arena_long(b.as.Long, a);
    }
    return b;
}

static arena _mod(arena a, arena b)
{

    switch (b.type)
    {
    case ARENA_CHAR:
        return mod_arena_char(b.as.Char, a);
    case ARENA_INT:
        return mod_arena_int(b.as.Int, a);
    case ARENA_LONG:
        return add_arena_long(b.as.Long, a);
    }

    return b;
}

static arena _eq(arena a, arena b)
{

    switch (b.type)
    {
    case ARENA_INT:
        return int_eq(b.as.Int, a);
    case ARENA_DOUBLE:
        return double_eq(b.as.Double, a);
    case ARENA_LONG:
        return long_eq(b.as.Long, a);
    case ARENA_CHAR:
        return char_eq(b.as.Char, a);
    case ARENA_STR:
        return string_eq(b, a);
    case ARENA_NULL:
        switch (a.type)
        {
        case ARENA_BOOL:
            return Bool(a.as.Bool ? false : true);
        case ARENA_NULL:
            return Bool(a.as.Bool == b.as.Bool);
        default:
            runtime_error("ERROR: Comparison type mismatch\n");
        }
    case ARENA_BOOL:
        switch (a.type)
        {
        case ARENA_BOOL:
            return Bool(a.as.Bool == b.as.Bool);
        case ARENA_NULL:
            return Bool(b.as.Bool ? false : true);
        default:
            runtime_error("ERROR: Comparison type mismatch\n");
        }
    }
    return b;
}
static arena _ne(arena a, arena b)
{

    switch (b.type)
    {
    case ARENA_INT:
        return int_ne(b.as.Int, a);
    case ARENA_DOUBLE:
        return double_ne(b.as.Double, a);
    case ARENA_LONG:
        return long_ne(b.as.Long, a);
    case ARENA_CHAR:
        return char_ne(b.as.Char, a);
    case ARENA_STR:
        return string_ne(b, a);
    case ARENA_NULL:
        switch (a.type)
        {
        case ARENA_BOOL:
            return Bool(a.as.Bool ? true : false);
        case ARENA_NULL:
            return Bool(a.as.Bool != b.as.Bool);
        default:
            runtime_error("ERROR: Comparison type mismatch\n");
        }
    case ARENA_BOOL:
        switch (a.type)
        {
        case ARENA_BOOL:
            return Bool(a.as.Bool != b.as.Bool);
        case ARENA_NULL:
            return Bool(b.as.Bool ? true : false);
        default:
            runtime_error("ERROR: Comparison type mismatch\n");
        }
    }
    return b;
}

static arena _lt(arena a, arena b)
{

    switch (b.type)
    {
    case ARENA_INT:
        return int_lt(b.as.Int, a);
    case ARENA_DOUBLE:
        return double_lt(b.as.Double, a);
    case ARENA_LONG:
        return long_lt(b.as.Long, a);
    case ARENA_CHAR:
        return char_lt(b.as.Char, a);
    case ARENA_STR:
        return string_lt(b, a);
    }
    return b;
}
static arena _le(arena a, arena b)
{

    switch (b.type)
    {
    case ARENA_INT:
        return int_le(b.as.Int, a);
    case ARENA_DOUBLE:
        return double_le(b.as.Double, a);
    case ARENA_LONG:
        return long_le(b.as.Long, a);
    case ARENA_CHAR:
        return char_le(b.as.Char, a);
    case ARENA_STR:
        return string_le(b, a);
    }
    return b;
}
static arena _gt(arena a, arena b)
{

    switch (b.type)
    {
    case ARENA_INT:
        return int_gt(b.as.Int, a);
    case ARENA_DOUBLE:
        return double_gt(b.as.Double, a);
    case ARENA_LONG:
        return long_gt(b.as.Long, a);
    case ARENA_CHAR:
        return char_gt(b.as.Char, a);
    case ARENA_STR:
        return string_gt(b, a);
    }
    return b;
}
static arena _ge(arena a, arena b)
{

    switch (b.type)
    {
    case ARENA_INT:
        return int_ge(b.as.Int, a);
    case ARENA_DOUBLE:
        return double_ge(b.as.Double, a);
    case ARENA_LONG:
        return long_ge(b.as.Long, a);
    case ARENA_CHAR:
        return char_ge(b.as.Char, a);
    case ARENA_STR:
        return string_ge(b, a);
    }
    return b;
}

static arena _or(arena a, arena b)
{
    return Bool(a.as.Bool || b.as.Bool);
}
static arena _and(arena a, arena b)
{
    return Bool(b.as.Bool && a.as.Bool);
}

static arena find(arena tmp)
{
    return find_entry(&machine.glob.map, &tmp);
}
static bool exists(arena tmp)
{
    return find_entry(&machine.glob.map, &tmp).type != ARENA_NULL;
}
static Interpretation undefined_var(arena tmp)
{
    runtime_error("Undefined variable `%s`.", tmp.as.String);
    return INTERPRET_RUNTIME_ERR;
}

Interpretation run()
{

#define READ_BYTE() (*machine.ip.as.Bytes++)
#define READ_CONSTANT() (machine.ch->constants.vals[READ_BYTE()])
#define POP() (*--machine.stack_top)

    uint8_t instruction;
    arena tmp;

    for (;;)
    {
#ifdef DEBUG_TRACE_EXECUTION
        for (arena *v = machine.stack; v < machine.stack_top; v++)
            print(*v);
        disassemble_instruction(machine.ch,
                                (int)(machine.ip.as.Bytes - machine.ch->op_codes.as.Bytes));
#endif

        switch (instruction = READ_BYTE())
        {
        case OP_CONSTANT:
            push(READ_CONSTANT());
            break;
        case OP_NEG:
            *--machine.stack_top = _neg(*machine.stack_top++);
            break;
        case OP_ADD:
            push(_add(POP(), POP()));
            break;
        case OP_POP:
            POP();
            break;
        case OP_SUB:
            push(_sub(POP(), POP()));
            break;
        case OP_MUL:
            push(_mul(POP(), POP()));
            break;
        case OP_MOD:
            push(_mod(POP(), POP()));
            break;
        case OP_DIV:
            push(_div(POP(), POP()));
            break;
        case OP_EQ:
            push(_eq(POP(), POP()));
            break;
        case OP_LT:
            push(_lt(POP(), POP()));
            break;
        case OP_LE:
            push(_le(POP(), POP()));
            break;
        case OP_GT:
            push(_gt(POP(), POP()));
            break;
        case OP_GE:
            push(_ge(POP(), POP()));
            break;
        case OP_NE:
            push(_ne(POP(), POP()));
            break;
        case OP_OR:
            push(_or(POP(), POP()));
            break;
        case OP_AND:
            push(_and(POP(), POP()));
            break;
        case OP_NULL:
            push(Null());
            break;
        case OP_SET_GLOBAL:
            tmp = READ_CONSTANT();
            if (!exists(tmp))
                return undefined_var(tmp);
            write_dict(&machine.glob, tmp, POP());
            push(find(tmp));
            break;
        case OP_GET_GLOBAL:
            push(find(READ_CONSTANT()));
            break;
        case OP_NOOP:
            break;
        case OP_GLOBAL_DEF:
            write_dict(&machine.glob, READ_CONSTANT(), POP());
            break;
        case OP_PRINT:
            tmp = POP();
            print(tmp.type == ARENA_VAR ? find(tmp) : tmp);
            break;
        case OP_RETURN:
            return INTERPRET_SUCCESS;
        }
    }

#undef POP
#undef READ_CONSTANT
#undef READ_BYTE
}
