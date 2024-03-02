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
static arena pop()
{
    machine.current_size--;
    return *--machine.stack_top;
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
    machine.ip_start = machine.ip.as.Bytes;
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

static void init_runtime(Runtime *runtime)
{
    runtime->true_count = 0;
    runtime->shorty = 0;
}

Interpretation run()
{

#define READ_BYTE() (*machine.ip.as.Bytes++)
#define READ_SHORT() ((uint16_t)((READ_BYTE() << 8) | READ_BYTE()))
#define READ_CONSTANT() (machine.ch->constants.vals[READ_BYTE()])
#define PEEK() (machine.stack_top[-1])
#define FALSEY() (!PEEK().as.Bool)
#define POP() \
    (--machine.current_size, *--machine.stack_top)
#define LOCAL() (machine.stack[READ_BYTE()])

    static Runtime runtime;
    init_runtime(&runtime);

    for (;;)
    {
#ifdef DEBUG_TRACE_EXECUTION
        for (arena *v = machine.stack; v < machine.stack_top; v++)
            print(*v);
        disassemble_instruction(machine.ch,
                                (int)(machine.ip.as.Bytes - machine.ch->op_codes.as.Bytes));
#endif

        switch (READ_BYTE())
        {
        case OP_CONSTANT:
            push(READ_CONSTANT());
            break;
        case OP_NEG:
            *--machine.stack_top = _neg(*machine.stack_top++);
            break;
        case OP_INC:
            push(_inc(POP()));
            break;
        case OP_DEC:
            push(_dec(POP()));
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
        case OP_NE:
            push(_ne(POP(), POP()));
            break;
        case OP_SEQ:
            push(_seq(POP(), POP()));
            break;
        case OP_SNE:
            push(_sne(POP(), POP()));
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
            machine.ip.as.Bytes += (READ_SHORT() * FALSEY());
            break;
        case OP_JMPL:
        {
            uint16_t offset = READ_SHORT();
            uint16_t jump = READ_SHORT();
            uint16_t index = READ_SHORT();

            if (FALSEY())
            {
                POP();
                machine.ip.as.Bytes += jump;
                break;
            }
            machine.ip.as.Bytes += offset;
            *(machine.ip.as.Bytes + jump) = ((index >> 8) & 0xFF);
            *(machine.ip.as.Bytes + jump + 1) = (index & 0xFF);
        }
        break;
        case OP_OFF_JMP:
        {
            uint16_t index = READ_SHORT();
            int len = machine.ch->cases.as.Ints[index];
            machine.ip.as.Bytes = machine.ip_start + len;
            machine.ch->case_count = 0;
        }
        break;
        case OP_JMPT:
            machine.ip.as.Bytes += (READ_SHORT() * !FALSEY());
            break;
        case OP_JMP:
            machine.ip.as.Bytes += READ_SHORT();
            break;
        case OP_LOOP:
            machine.ip.as.Bytes -= READ_SHORT();
            break;
        case OP_GET_LOCAL:
            push(LOCAL());
            break;
        case OP_SET_LOCAL:
            LOCAL() = PEEK();
            break;
        case OP_SET_GLOBAL:
        {
            arena ar = READ_CONSTANT();
            if (!exists(ar))
                return undefined_var(ar);
            write_dict(&machine.glob, ar, POP());
            push(find(ar));
            break;
        }
        case OP_GET_GLOBAL:
            push(find(READ_CONSTANT()));
            break;
        case OP_NOOP:
            break;
        case OP_GLOBAL_DEF:
            write_dict(&machine.glob, READ_CONSTANT(), POP());
            break;
        case OP_PRINT:
            print(POP());
            break;
        case OP_RETURN:
            return INTERPRET_SUCCESS;
        }
    }

#undef LOCAL
#undef POP
#undef FALSEY
#undef PEEK
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_BYTE
}
