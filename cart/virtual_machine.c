#include "virtual_machine.h"
#include "vm_util.h"
#include "common.h"
#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

void initVM()
{

    initialize_global_memory(PAGE);
    machine.stack = GROW_STACK(NULL, (size_t)ARENA_SIZE);
    init_dict(&machine.glob);
    define_native(native_name("clock"), clock_native);
    define_native(native_name("square"), square_native);
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
        Function *func = frame->closure->func;
        size_t instruction = frame->ip - frame->closure->func->ch.op_codes.listof.Bytes - 1;
        int line = frame->closure->func->ch.line;

        if (!func->name.as.String)
            fprintf(stderr, "script\n");
        else
            fprintf(stderr, "%s()\n", func->name.as.String);
        fprintf(stderr, "[line %d] in script\n", line);
    }
    reset_stack(machine.stack);
}

static void define_native(arena ar, NativeFn n)
{
    ar.as.hash %= machine.glob.capacity;
    insert_entry(&machine.glob.map, native_entry(native(n, ar)));
}

static Element clock_native(int argc, Stack *args)
{
    return OBJ(Double(clock() / CLOCKS_PER_SEC));
}

static arena _sqr(arena a)
{
    switch (a.type)
    {
    case ARENA_INT:
        return Int((int)sqrt(a.as.Int));
    case ARENA_DOUBLE:
        return Int((int)sqrt(a.as.Double));
    case ARENA_LONG:
        return Int((int)sqrt(a.as.Long));
    case ARENA_BYTE:
        return Int((int)sqrt(a.as.Byte));
    case ARENA_CHAR:
        return Int((int)sqrt(a.as.Char));
    }
}

static Element square_native(int argc, Stack *args)
{
    return OBJ(_sqr(args->as.arena));
}

static bool call(Closure *c, uint8_t argc)
{

    if (c->func->arity != argc)
    {

        runtime_error("Expected `%d` args, but got `%d`.", c->func->arity, argc);
        return false;
    }

    if (machine.frame_count == FRAMES_MAX)
    {
        runtime_error("Stack overflow.");
        return false;
    }

    CallFrame *frame = &machine.frames[machine.frame_count++];
    frame->closure = c;
    frame->ip = c->func->ch.op_codes.listof.Bytes;
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

    Closure *clos = new_closure(func);
    call(clos, 0);
    push(&machine.stack, closure(clos));

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
static Native *find_native(arena hash)
{
    hash.as.hash %= machine.glob.capacity;
    return find_native_entry(&machine.glob.map, &hash);
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
    case CLOSURE:
        return call(el.closure, argc);
    case NATIVE:
    {
        Element res = el.native->fn(argc, machine.stack->top - argc);
        machine.stack->count -= (argc + 1);
        machine.stack->top -= (argc + 1);
        push(&machine.stack, res);
        return true;
    }
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
#define READ_SHORT() (((READ_BYTE() << 8) & 0xFF) | READ_BYTE() & 0xFF)
#define READ_CONSTANT() (frame->closure->func->ch.constants[READ_BYTE()].as)
#define PEEK() (machine.stack->top[-1].as)
#define NPEEK(N) (machine.stack->top[(-1 - N)].as)
#define FALSEY() (!PEEK().arena.as.Bool)
#define POPN(n) (popn(&machine.stack, n))
#define LOCAL() (frame->slots[READ_BYTE()].as)
#define JUMP() (frame->closure->func->ch.cases.listof.Ints[READ_SHORT()])
#define PUSH(ar) (push(&machine.stack, ar))
#define FIND_FUNC(ar) (find_func(ar))
#define FIND_NATIVE(ar) (find_native(ar))
#define WRITE_AR(a, b) (write_dict(&machine.glob, a, b, machine.glob.capacity))
#define WRITE_FUNC(f) (write_func_dict(&machine.glob, f, machine.glob.capacity))
#define POP() \
    (--machine.stack->count, (*--machine.stack->top).as)

    for (;;)
    {
#ifdef DEBUG_TRACE_EXECUTION
        for (Stack *v = machine.stack; v < machine.stack->top; v++)
            print(v->as);
        disassemble_instruction(&frame->closure->func->ch,
                                (int)(frame->ip - frame->closure->func->ch.op_codes.listof.Bytes));
#endif

        switch (READ_BYTE())
        {
        case OP_CONSTANT:
            PUSH(READ_CONSTANT());
            break;
        case OP_CLOSURE:
            PUSH(READ_CONSTANT());
            break;
        case OP_NEG:
            (*--machine.stack->top).as = OBJ(_neg((*machine.stack->top++).as.arena));
            break;
        case OP_INC:
            (*--machine.stack->top).as = OBJ(_inc((*machine.stack->top++).as.arena));
            break;
        case OP_INC_LOC:
        {
            uint8_t index = READ_BYTE();
            Element el = OBJ(_inc(frame->slots[index].as.arena));
            frame->slots[index].as = el;
            PUSH(el);
            break;
        }
        case OP_INC_GLO:
        {
            Element key = READ_CONSTANT();
            Element ar = OBJ(_inc(find(key.arena)));
            WRITE_AR(key.arena, ar.arena);
            PUSH(ar);
            break;
        }
        case OP_DEC_LOC:
        {
            uint8_t index = READ_BYTE();
            Element el = OBJ(_dec(frame->slots[index].as.arena));
            frame->slots[index].as = el;
            PUSH(el);
            break;
        }
        case OP_DEC_GLO:
        {
            Element key = READ_CONSTANT();
            Element ar = OBJ(_dec(find(key.arena)));
            WRITE_AR(key.arena, ar.arena);
            PUSH(ar);
            break;
        }
        case OP_DEC:
            (*--machine.stack->top).as = OBJ(_dec((*machine.stack->top++).as.arena));
            break;
        case OP_ADD:
            PUSH(OBJ(_add(POP().arena, POP().arena)));
            break;
        case OP_POPN:
            POPN(READ_CONSTANT().arena.as.Int);
            break;
        case OP_POP:
            pop();
            break;
        case OP_SUB:
            PUSH(OBJ(_sub(POP().arena, POP().arena)));
            break;
        case OP_MUL:
            PUSH(OBJ(_mul(POP().arena, POP().arena)));
            break;
        case OP_MOD:
            PUSH(OBJ(_mod(POP().arena, POP().arena)));
            break;
        case OP_DIV:
            PUSH(OBJ(_div(POP().arena, POP().arena)));
            break;
        case OP_EQ:
            PUSH(OBJ(_eq(POP().arena, POP().arena)));
            break;
        case OP_NE:
            PUSH(OBJ(_ne(POP().arena, POP().arena)));
            break;
        case OP_SEQ:
            PUSH(OBJ(_seq(POP().arena, POP().arena)));
            break;
        case OP_SNE:
            PUSH(OBJ(_sne(POP().arena, POP().arena)));
            break;
        case OP_LT:
            PUSH(OBJ(_lt(POP().arena, POP().arena)));
            break;
        case OP_LE:
            PUSH(OBJ(_le(POP().arena, POP().arena)));
            break;
        case OP_GT:
            PUSH(OBJ(_gt(POP().arena, POP().arena)));
            break;
        case OP_GE:
            PUSH(OBJ(_ge(POP().arena, POP().arena)));
            break;
        case OP_OR:
            PUSH(OBJ(_or(POP().arena, POP().arena)));
            break;
        case OP_AND:
            PUSH(OBJ(_and(POP().arena, POP().arena)));
            break;
        case OP_NULL:
            PUSH(OBJ(Null()));
            break;
        case OP_JMPF:
            frame->ip += (READ_SHORT() * FALSEY());
            break;
        case OP_JMPC:
        {
            uint16_t jump = READ_SHORT();
            uint16_t offset = READ_SHORT();

            if (FALSEY())
            {
                POP();
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
            frame->ip += (READ_SHORT() * !FALSEY());
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
                    PUSH(CLOSURE(new_closure(f)));
            }
            else if (el.type == ARENA && el.arena.type == ARENA_NATIVE)
            {
                Native *n;
                if ((n = FIND_NATIVE(el.native->obj)) != NULL)
                    PUSH(NATIVE(n));
            }
            else
                PUSH(el);
            break;
        }
        case OP_SET_LOCAL:
        {
            Element el = PEEK();
            if (el.type == CLOSURE)
                WRITE_FUNC(el.closure->func);
            // else
            LOCAL() = el;
            break;
        }
        case OP_SET_GLOBAL:
        {
            Element el = READ_CONSTANT();
            if (el.type != CLOSURE)
            {
                if (!exists(el.arena))
                    return undefined_var(el.arena);
                arena ar = POP().arena;
                WRITE_AR(el.arena, ar);
                PUSH(OBJ(ar));
            }
            else
                WRITE_FUNC(el.closure->func);
        }
        break;
        case OP_GET_GLOBAL:
        {
            Element el = READ_CONSTANT();

            if (el.type == ARENA && el.arena.type == ARENA_FUNC)
            {
                Function *f;
                if ((f = FIND_FUNC(el.arena)) != NULL)
                    PUSH(CLOSURE(new_closure(f)));
            }
            else if (el.type == ARENA && el.arena.type == ARENA_NATIVE)
            {
                Native *native;
                if ((native = FIND_NATIVE(el.arena)) != NULL)
                    PUSH(NATIVE(native));
            }
            else
                PUSH(OBJ(find(el.arena)));
        }
        break;
        case OP_NOOP:
            break;
        case OP_GLOBAL_DEF:
        {
            Element el = READ_CONSTANT();
            if (el.type == CLOSURE)
                WRITE_FUNC(el.closure->func);
            else
                WRITE_AR(el.arena, POP().arena);
            break;
        }
        case OP_PRINT:
        {
            Element el = POP();
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

            Element el = POP();
            --machine.frame_count;

            if (machine.frame_count == 0)
            {
                POP();
                return INTERPRET_SUCCESS;
            }

            for (Stack *s = machine.stack; s < frame->slots; s++)
                POP();

            machine.stack->top = frame->slots;
            PUSH(el);

            frame = &machine.frames[machine.frame_count - 1];
            break;
        }
        }
    }
#undef POP
#undef WRITE_FUNC
#undef WRITE_AR
#undef FIND_NATIVE
#undef FIND_FUNC
#undef PUSH
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
