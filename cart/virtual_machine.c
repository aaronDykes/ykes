#include "compiler.h"
#include "vm_util.h"
#include <stdarg.h>
#include <stdio.h>

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

static void push(arena ar)
{
    check_stack_size();
    machine.current_size++;
    *machine.stack_top++ = ar;
}

static void popn(arena n)
{
    for (int i = 0; i < n.as.Int; i++)
        --machine.current_size, --machine.stack_top;
}

Interpretation interpret(const char *src)
{
    chunk ch;
    init_chunk(&ch);

    if (!compile(src, &ch, &machine))
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
static uint8_t falsey()
{
    return (!machine.stack_top[-1].as.Bool) ? 1 : 0;
}

Interpretation run()
{

#define READ_BYTE() (*machine.ip.as.Bytes++)
#define READ_SHORT() ((uint16_t)((READ_BYTE() << 8) | READ_BYTE()))
#define READ_CONSTANT() (machine.ch->constants.vals[READ_BYTE()])
#define POP() \
    (--machine.current_size, *--machine.stack_top)
#define LOCAL() (machine.stack[READ_BYTE()])

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
        case OP_POPN:
            popn(READ_CONSTANT());
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
        case OP_JMPF:
            machine.ip.as.Bytes += (READ_SHORT() * falsey());
            break;
        case OP_JMP:
            machine.ip.as.Bytes += READ_SHORT();
            break;
        case OP_GET_LOCAL:
            push(LOCAL());
            break;
        case OP_SET_LOCAL:
            LOCAL() = machine.stack_top[-1];
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

#undef LOCAL
#undef POP
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_BYTE
}
