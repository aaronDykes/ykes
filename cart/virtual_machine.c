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

    for (int i = machine.frame_count - 1; i >= 0; i--)
    {

        CallFrame *frame = &machine.frames[i];
        Function *func = frame->func;
        size_t instruction = frame->ip - frame->func->ch.op_codes.listof.Bytes - 1;
        int line = frame->func->ch.line;

        if (!func->name.as.String)
            fprintf(stderr, "script\n");
        else
            fprintf(stderr, "%s()\n", func->name.as.String);
        fprintf(stderr, "[line %d] in script\n", line);
    }
    reset_stack(machine.stack);
}

static bool call(Function *f, uint8_t argc)
{

    if (f->arity != argc)
    {

        runtime_error("Expected `%d` args, but got `%d`.", f->arity, argc);
        return false;
    }

    if (machine.frame_count == FRAMES_MAX)
    {
        runtime_error("Stack overflow.");
        return false;
    }

    CallFrame *frame = &machine.frames[machine.frame_count++];
    frame->func = f;
    frame->ip = f->ch.op_codes.listof.Bytes;
    frame->ip_start = frame->ip;
    frame->slots = machine.stack->top - argc - 1;
    return true;
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
    call(func, 0);

    Interpretation res = run();
    return res;
}

static arena find(arena tmp)
{
    tmp.as.hash %= machine.glob.capacity;
    return find_arena_entry(&machine.glob.map, &tmp);
}

static Function *find_func(arena hash)
{
    hash.as.hash %= machine.glob.capacity;
    return find_func_entry(&machine.glob.map, &hash);
}

static bool exists(arena tmp)
{
    tmp.as.hash %= machine.glob.capacity;
    return find_arena_entry(&machine.glob.map, &tmp).type != ARENA_NULL;
}

static Function *func_exists(arena ar)
{
    ar.as.hash %= machine.glob.capacity;
    return find_func_entry(&machine.glob.map, &ar);
}

static Interpretation undefined_var(arena tmp)
{
    runtime_error("Undefined variable `%s`.", tmp.as.String);
    return INTERPRET_RUNTIME_ERR;
}
static Interpretation duplicate_func(arena tmp)
{
    runtime_error("Duplicate function `%s`.", tmp.as.String);
    return INTERPRET_RUNTIME_ERR;
}

static bool call_value(Element el, uint8_t argc)
{
    switch (el.type)
    {
    case FUNC:
        return call(el.func, argc);
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
#define NPEEK(N) (machine.stack->top[(-1 - N)].as)
#define FALSEY() (!PEEK().arena.as.Bool)
#define POPN(n) (popn(&machine.stack, n))
#define LOCAL() (frame->slots[READ_BYTE()].as)
#define JUMP() (frame->func->ch.cases.listof.Ints[READ_SHORT()])
#define OBJ(ar) (Obj(ar))
#define PUSH(ar) (push(&machine.stack, ar))
#define FIND_FUNC(ar) (find_func(ar))
#define WRITE_AR(a, b) (write_dict(&machine.glob, a, b, machine.glob.capacity))
#define WRITE_FUNC(a, f) (write_func_dict(&machine.glob, a, f, machine.glob.capacity))

    for (;;)
    {
#ifdef DEBUG_TRACE_EXECUTION
        for (Stack *v = machine.stack; v < machine.stack->top; v++)
            print(v->as);
        disassemble_instruction(&frame->func->ch,
                                (int)(frame->ip - frame->func->ch.op_codes.listof.Bytes));
#endif

        switch (READ_BYTE())
        {
        case OP_CONSTANT:
            PUSH(READ_CONSTANT());
            break;
        case OP_NEG:
            PUSH(Obj(_neg(pop().arena)));
            break;
        case OP_INC:
            PUSH(Obj(_inc(pop().arena)));
            break;
        case OP_DEC:
            PUSH(Obj(_dec(pop().arena)));
            break;
        case OP_ADD:
            PUSH(Obj(_add(pop().arena, pop().arena)));
            break;
        case OP_POPN:
            POPN(READ_CONSTANT().arena.as.Int);
            break;
        case OP_POP:
            pop();
            break;
        case OP_SUB:
            PUSH(Obj(_sub(pop().arena, pop().arena)));
            break;
        case OP_MUL:
            PUSH(Obj(_mul(pop().arena, pop().arena)));
            break;
        case OP_MOD:
            PUSH(Obj(_mod(pop().arena, pop().arena)));
            break;
        case OP_DIV:
            PUSH(Obj(_div(pop().arena, pop().arena)));
            break;
        case OP_EQ:
            PUSH(Obj(_eq(pop().arena, pop().arena)));
            break;
        case OP_NE:
            PUSH(Obj(_ne(pop().arena, pop().arena)));
            break;
        case OP_SEQ:
            PUSH(Obj(_seq(pop().arena, pop().arena)));
            break;
        case OP_SNE:
            PUSH(Obj(_sne(pop().arena, pop().arena)));
            break;
        case OP_LT:
            PUSH(Obj(_lt(pop().arena, pop().arena)));
            break;
        case OP_LE:
            PUSH(Obj(_le(pop().arena, pop().arena)));
            break;
        case OP_GT:
            PUSH(Obj(_gt(pop().arena, pop().arena)));
            break;
        case OP_GE:
            PUSH(Obj(_ge(pop().arena, pop().arena)));
            break;
        case OP_OR:
            PUSH(Obj(_or(pop().arena, pop().arena)));
            break;
        case OP_AND:
            PUSH(Obj(_and(pop().arena, pop().arena)));
            break;
        case OP_NULL:
            PUSH(Obj(Null()));
            break;
        case OP_JMPF:
            frame->ip += (READ_SHORT() * FALSEY());
            break;
        case OP_JMPC:
        {
            uint16_t jump = READ_SHORT(), offset = READ_SHORT();

            if (FALSEY())
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
            Element el = NPEEK(argc);

            if (!call_value(el, argc))
                return INTERPRET_RUNTIME_ERR;

            frame = &machine.frames[machine.frame_count - 1];
        }
        break;
        case OP_JMPT:
            frame->ip += (READ_SHORT() * FALSEY());
            break;
        case OP_JMP:
            frame->ip += READ_SHORT();
            break;
        case OP_LOOP:
            frame->ip -= READ_SHORT();
            break;
        case OP_GET_LOCAL:
        {
            Element el = LOCAL();
            if (el.type == ARENA && el.arena.type == ARENA_FUNC)
            {
                Function *f;
                if ((f = FIND_FUNC(el.func->name)) != NULL)
                    PUSH(Func(f));
            }
            else
            {

                PUSH(el);
            }
            break;
        }
        case OP_SET_LOCAL:
        {
            Element el = PEEK();
            if (el.type == FUNC)
            {
                WRITE_FUNC(el.func->name, el.func);
            }
            else
                LOCAL() = el;
            break;
        }
        case OP_SET_GLOBAL:
        {
            Element el = READ_CONSTANT();
            if (el.type == ARENA)
            {
                if (!exists(el.arena))
                    return undefined_var(el.arena);
                WRITE_AR(el.arena, pop().arena);

                PUSH(Obj(find(el.arena)));
            }
            else if (el.type == FUNC)
            {
                WRITE_FUNC(el.func->name, el.func);
            }
        }
        break;
        case OP_GET_GLOBAL:
        {
            Element el = READ_CONSTANT();

            if (el.type == ARENA && el.arena.type == ARENA_FUNC)
            {
                Function *f;
                if ((f = FIND_FUNC(el.arena)) != NULL)
                    PUSH(Func(f));
            }
            else
                PUSH(Obj(find(el.arena)));
        }
        break;
        case OP_NOOP:
            break;
        case OP_GLOBAL_DEF:
        {
            Element el = READ_CONSTANT();
            if (el.type == FUNC)
            {
                pop();
                WRITE_FUNC(el.func->name, el.func);
            }
            else
                WRITE_AR(el.arena, pop().arena);
            break;
        }
        case OP_PRINT:
        {

            Element el = pop();
            if (el.type == FUNC)
            {
                PUSH(el);
                break;
            }
            print(el);
            break;
        }
        case OP_RETURN:
        {

            Element el = pop();
            --machine.frame_count;

            if (machine.frame_count == 0)
            {
                pop();
                return INTERPRET_SUCCESS;
            }

            for (Stack *s = machine.stack; s < frame->slots; s++)
                pop();

            machine.stack->top = frame->slots;
            PUSH(el);

            frame = &machine.frames[machine.frame_count - 1];
            break;
        }
        }
    }
#undef WRITE_FUNC
#undef WRITE_AR
#undef FIND_FUNC
#undef PUSH
#undef OBJ
#undef JUMP
#undef LOCAL
#undef POPN
#undef POP
#undef FALSEY
#undef NPEEK
#undef PEEK
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_BYTE
}
