#include "virtual_machine.h"
#include "vm_util.h"
#include "common.h"
#include <stdarg.h>
#include <stdio.h>

void initVM()
{

    initialize_global_memory(PAGE);
    machine.stack = GROW_STACK(NULL, (size_t)STACK_SIZE);
    init_dict(&machine.glob);
}
void freeVM()
{
    free_dict(&machine.glob);
    FREE_STACK(machine.stack);
    destroy_global_memory();
}

static void runtime_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
    CallFrame *frame = &machine.frames[machine.frame_count - 1];
    size_t instruction = frame->ip - frame->func->ch.op_codes.listof.Bytes - 1;
    int line = frame->func->ch.line;
    fprintf(stderr, "[line %d] in script\n", line);
    reset_stack(machine.stack);
}

Interpretation interpret(const char *src)
{

    Function *func = NULL;
    if (!(func = compile(src)))
    {
        free_function(func);
        return INTERPRET_COMPILE_ERR;
    }

    push(&machine.stack, Func(func));
    CallFrame *frame = &machine.frames[machine.frame_count++];
    frame->func = func;
    frame->ip = func->ch.op_codes.listof.Bytes;
    frame->ip_start = func->ch.op_codes.listof.Bytes;
    frame->slots = machine.stack;

    Interpretation res = run();
    free_function(func);
    return res;
}

static arena find(arena tmp)
{
    tmp.as.hash %= machine.glob.capacity;
    return find_arena_entry(&machine.glob.map, &tmp);
}

static bool exists(arena tmp)
{
    tmp.as.hash %= machine.glob.capacity;
    return find_arena_entry(&machine.glob.map, &tmp).type != ARENA_NULL;
}
static Interpretation undefined_var(arena tmp)
{
    runtime_error("Undefined variable `%s`.", tmp.as.String);
    return INTERPRET_RUNTIME_ERR;
}

static bool call(Function *f, uint8_t argc)
{
}

static bool call_value(Function *f, uint8_t argc)
{
    switch (f->name.type)
    {
    case ARENA_FUNC:
        return call(f, argc);
    default:
        break;
    }
    runtime_error("Can only call functions and classes.");
    return false;
}

static Element pop()
{
    --machine.stack->count;
    return (*--machine.stack->top).as;
}

Interpretation run()
{

    CallFrame *frame = &machine.frames[machine.frame_count - 1];
#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() ((uint16_t)((READ_BYTE() << 8) | READ_BYTE()))
#define READ_CONSTANT() (frame->func->ch.constants[READ_BYTE()].as)
#define PEEK() (machine.stack->top[-1].as)
#define NPEEK(N) (machine.stack->top[N].as)
#define FALSEY() (PEEK().arena)
#define POP() \
    (--machine.stack->count, (*--machine.stack->top).as)
#define LOCAL() (frame->slots[READ_BYTE()].as)
#define JUMP() (frame->func->ch.cases.listof.Ints[READ_SHORT()])

    for (;;)
    {
#ifdef DEBUG_TRACE_EXECUTION
        for (Stack *v = machine.stack + 1; v < machine.stack->top; v++)
            print(v->as);
        disassemble_instruction(&frame->func->ch,
                                (int)(frame->ip - frame->func->ch.op_codes.listof.Bytes));
#endif

        switch (READ_BYTE())
        {
        case OP_CONSTANT:
            push(&machine.stack, READ_CONSTANT());
            break;
        case OP_NEG:
            (*--machine.stack->top).as = Obj(_neg((*machine.stack++).as.arena));
            break;
        case OP_INC:
            push(&machine.stack, Obj(_inc(pop().arena)));
            break;
        case OP_DEC:
            push(&machine.stack, Obj(_dec(pop().arena)));
            break;
        case OP_ADD:
            push(&machine.stack, Obj(_add(pop().arena, pop().arena)));
            break;
        case OP_POPN:
            popn(&machine.stack, READ_CONSTANT().arena.as.Int);
            break;
        case OP_POP:
            pop();
            break;
        case OP_SUB:
            push(&machine.stack, Obj(_sub(pop().arena, pop().arena)));
            break;
        case OP_MUL:
            push(&machine.stack, Obj(_mul(pop().arena, pop().arena)));
            break;
        case OP_MOD:
            push(&machine.stack, Obj(_mod(pop().arena, pop().arena)));
            break;
        case OP_DIV:
            push(&machine.stack, Obj(_div(pop().arena, pop().arena)));
            break;
        case OP_EQ:
            push(&machine.stack, Obj(_eq(pop().arena, pop().arena)));
            break;
        case OP_NE:
            push(&machine.stack, Obj(_ne(pop().arena, pop().arena)));
            break;
        case OP_SEQ:
            push(&machine.stack, Obj(_seq(pop().arena, pop().arena)));
            break;
        case OP_SNE:
            push(&machine.stack, Obj(_sne(pop().arena, pop().arena)));
            break;
        case OP_LT:
            push(&machine.stack, Obj(_lt(pop().arena, pop().arena)));
            break;
        case OP_LE:
            push(&machine.stack, Obj(_le(pop().arena, pop().arena)));
            break;
        case OP_GT:
            push(&machine.stack, Obj(_gt(pop().arena, pop().arena)));
            break;
        case OP_GE:
            push(&machine.stack, Obj(_ge(pop().arena, pop().arena)));
            break;
        case OP_OR:
            push(&machine.stack, Obj(_or(pop().arena, pop().arena)));
            break;
        case OP_AND:
            push(&machine.stack, Obj(_and(pop().arena, pop().arena)));
            break;
        case OP_NULL:
            push(&machine.stack, Obj(Null()));
            break;
        case OP_JMPF:
        {

            arena ar = FALSEY();
            uint16_t offset = READ_SHORT();
            frame->ip += (offset * !ar.as.Bool);
            break;
        }
        case OP_JMPC:
        {
            arena ar = FALSEY();
            uint16_t jump = READ_SHORT(), offset = READ_SHORT();

            if (!ar.as.Bool)
            {
                pop();
                frame->ip += jump;
                break;
            }
            frame->ip += offset;
        }
        break;
        case OP_JMPL:
            frame->ip = frame->ip_start + JUMP();
            break;
        case OP_CALL:
        {
            uint8_t argc = READ_BYTE();
            if (!call_value(NPEEK(argc).func, argc))
                return INTERPRET_RUNTIME_ERR;
            frame = &machine.frames[machine.frame_count - 1];
        }
        break;
        case OP_JMPT:
        {
            arena ar = FALSEY();

            frame->ip += (READ_SHORT() * !ar.as.Bool);
            break;
        }
        case OP_JMP:
            frame->ip += READ_SHORT();
            break;
        case OP_LOOP:
            frame->ip -= READ_SHORT();
            break;
        case OP_GET_LOCAL:
            push(&machine.stack, LOCAL());
            break;
        case OP_SET_LOCAL:
            LOCAL() = PEEK();
            break;
        case OP_SET_GLOBAL:
        {
            Element el = READ_CONSTANT();
            // if (el.type == ARENA)
            // {
            if (!exists(el.arena))
                return undefined_var(el.arena);
            write_dict(
                &machine.glob,
                el.arena,
                pop().arena,
                machine.glob.capacity);

            push(&machine.stack, Obj(find(el.arena)));
            // }
            // else
            // push(&machine.stack, el);
            break;
        }
        case OP_GET_GLOBAL:
        {

            // Element el = READ_CONSTANT();
            // if (el.type == ARENA)
            push(&machine.stack, Obj(find(READ_CONSTANT().arena)));
            // else
            // push(&machine.stack, el);
        }
        break;
        case OP_NOOP:
            break;
        case OP_GLOBAL_DEF:
        {
            // Element el = READ_CONSTANT();
            // if (el.type == ARENA && el.arena.type == ARENA_FUNC)
            //     write_func_dict(
            //         &machine.glob,
            //         el.arena,
            //         READ_CONSTANT().func,
            //         machine.glob.capacity);
            write_dict(
                &machine.glob,
                READ_CONSTANT().arena,
                pop().arena,
                machine.glob.capacity);
        }
        break;
        case OP_PRINT:
            print(pop());
            break;
        case OP_RETURN:
            return INTERPRET_SUCCESS;
        }
    }
#undef JUMP
#undef LOCAL
#undef POP
#undef FALSEY
#undef NPEEK
#undef PEEK
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_BYTE
}
