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
    machine.stack = GROW_STACK(NULL, STACK_SIZE);
    machine.call_stack = GROW_STACK(NULL, STACK_SIZE);
    machine.native_calls = GROW_STACK(NULL, NATIVE_STACK_SIZE);
    init_dict(&machine.glob);
    define_native(native_name("clock"), clock_native);
    define_native(native_name("square"), square_native);
    define_native(native_name("prime"), prime_native);
}
void freeVM()
{
    free_dict(&machine.glob);
    FREE_STACK(machine.stack);
    FREE_STACK(machine.call_stack);
    FREE_STACK(machine.native_calls);
    destroy_global_memory();
}

static void reset_vm_stack()
{
    reset_stack(machine.stack);
    machine.frame_count = 0;
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
        int line = frame->closure->func->ch.line;

        if (!func->name.as.String)
            fprintf(stderr, "script\n");
        else
            fprintf(stderr, "%s()\n", func->name.as.String);
        fprintf(stderr, "[line %d] in script\n", line);
    }

    reset_vm_stack();
}

static void define_native(Arena ar, NativeFn n)
{
    Element el = native_fn(native(n, ar));
    push(&machine.native_calls, el);
}

static inline Element prime_native(int argc, Stack *args)
{
    return OBJ(_prime(args->as.arena));
}

static inline Element clock_native(int argc, Stack *args)
{
    return OBJ(Double((double)clock() / CLOCKS_PER_SEC));
}

static inline Element square_native(int argc, Stack *args)
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
    frame->closure->upvals = c->upvals;
    frame->ip = c->func->ch.op_codes.listof.Bytes;
    frame->ip_start = c->func->ch.op_codes.listof.Bytes;
    frame->slots = machine.stack->top - argc - 1;
    return true;
}

Interpretation interpret(const char *src)
{

    Function *func = NULL;

    if (!(func = compile(src)))
        return INTERPRET_RUNTIME_ERR;

    Closure *clos = new_closure(func);
    call(clos, 0);

    push(&machine.stack, closure(clos));

    close_upvalues(machine.stack->top - 1);
    Interpretation res = run();
    return res;
}

static Arena find(Arena tmp)
{
    tmp.as.hash %= machine.glob.capacity;
    return find_arena_entry(&machine.glob.map, &tmp);
}

static Closure *find_func(Arena hash)
{
    hash.as.hash %= machine.glob.capacity;
    return find_func_entry(&machine.glob.map, &hash);
}
static Native *find_native(Arena hash)
{
    hash.as.hash %= machine.glob.capacity;
    return find_native_entry(&machine.glob.map, &hash);
}

static Interpretation undefined_var(Arena tmp)
{
    runtime_error("Undefined variable `%s`.", tmp.as.String);
    return INTERPRET_RUNTIME_ERR;
}

static bool exists(Arena tmp)
{
    tmp.as.hash %= machine.glob.capacity;
    return find_arena_entry(&machine.glob.map, &tmp).type != ARENA_NULL;
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

static Upval *capture_upvalue(Stack *s)
{
    Upval *prev = NULL;
    Upval *curr = machine.open_upvals;

    for (; curr && curr->index < s; curr = curr->next)
        prev = curr;

    if (curr && curr->index == s)
        return curr;

    Upval *new = upval(s);
    new->next = curr;

    if (prev)
        prev->next = new;
    else
        machine.open_upvals = new;
    return new;
}

static void close_upvalues(Stack *local)
{
    while (machine.open_upvals && machine.open_upvals->index > local)
    {
        Upval *up = machine.open_upvals;
        up->closed = *machine.open_upvals->index;
        up->index = &up->closed;
        machine.open_upvals = up->next;
    }
}
Closure *traverse_stack_closure(Arena *ar)
{
    for (Stack *s = machine.call_stack; s < machine.call_stack->top; s++)
        if (s->as.closure->func->name.as.hash == ar->as.hash)
            return s->as.closure;
    return NULL;
}

Native *traverse_stack_native(Arena *ar)
{
    for (Stack *s = machine.native_calls->top - 1; s >= machine.native_calls; s--)
        if (s->as.native->obj.as.hash == ar->as.hash)
            return s->as.native;
    return NULL;
}

Interpretation run()
{

    CallFrame *frame = &machine.frames[machine.frame_count - 1];

#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() (((READ_BYTE() << 8) & 0xFF) | READ_BYTE() & 0xFF)
#define READ_CONSTANT() (frame->closure->func->ch.constants[READ_BYTE()].as)
#define GET_FUNC(ar) \
    (traverse_stack_closure(&ar))
#define GET_NATIVE(ar) \
    (NATIVE(traverse_stack_native(&ar)))
#define PEEK() ((machine.stack->top - 1)->as)
#define NPEEK(N) ((machine.stack->top + (-1 + -N))->as)
#define CPEEK(N) ((machine.call_stack->top + (-1 + -N))->as)
#define FALSEY() (!PEEK().arena.as.Bool)
#define POPN(n) (popn(&machine.stack, n))
#define LOCAL() ((frame->slots + READ_BYTE())->as)
#define JUMP() (*(frame->closure->func->ch.cases.listof.Ints + READ_SHORT()))
#define PUSH(ar) (push(&machine.stack, ar))
#define CPUSH(ar) (push(&machine.call_stack, ar))
#define FIND_FUNC(ar) (find_func(ar))
#define FIND_NATIVE(ar) (find_native(ar))
#define WRITE_AR(a, b) (write_dict(&machine.glob, a, b, machine.glob.capacity))
#define WRITE_FUNC(f) (write_func_dict(&machine.glob, f, machine.glob.capacity))
#define POP() \
    (--machine.stack->count, (--machine.stack->top)->as)
#define CPOP() \
    (--machine.call_stack->count, (--machine.call_stack->top)->as)

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
        {
            Closure *c = READ_CONSTANT().closure;
            CPUSH(CLOSURE(c));

            for (int i = 0; i < c->upval_count; i++)
            {
                uint8_t is_local = READ_BYTE();
                uint8_t index = READ_BYTE();

                if (is_local)
                    frame->closure->upvals[i].index = capture_upvalue(frame->slots + index)->index;
                else
                    frame->closure->upvals[i].index = c->upvals[i].index;
            }
        }
        break;
        case OP_GET_UPVALUE:
        {
            Stack s = *frame->closure->upvals[READ_BYTE()].index;

            PUSH(s.as);
            break;
        }
        case OP_SET_UPVALUE:
            *frame->closure->upvals[READ_BYTE()].index = *(machine.stack->top - 1);
            break;
        case OP_NEG:
            (--machine.stack->top)->as = OBJ(_neg((machine.stack->top++)->as.arena));
            break;
        case OP_INC:
            (--machine.stack->top)->as = OBJ(_inc((machine.stack->top++)->as.arena));
            break;
        case OP_INC_LOC:
        {
            uint8_t index = READ_BYTE();
            Element el = OBJ(_inc((frame->slots + index)->as.arena));
            (frame->slots + index)->as = el;
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
            Element el = OBJ(_dec((frame->slots + index)->as.arena));
            (frame->slots + index)->as = el;
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
            (--machine.stack->top)->as = OBJ(_dec((machine.stack->top++)->as.arena));
            break;
        case OP_ADD:
            PUSH(OBJ(_add(POP().arena, POP().arena)));
            break;
        case OP_POPN:
            POPN(READ_CONSTANT().arena.as.Int);
            break;
        case OP_POP:
            POP();
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
            if (!call_value(NPEEK(argc), argc))
                return INTERPRET_RUNTIME_ERR;

            frame = (machine.frames + (machine.frame_count - 1));
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
        case OP_CLOSE_UPVAL:
            close_upvalues(machine.stack->top - 1);
            POP();
            break;
        case OP_GET_LOCAL:
            PUSH(LOCAL());
            break;
        case OP_SET_LOCAL:
            LOCAL() = PEEK();
            break;
        case OP_SET_GLOBAL:
        {
            Element el = READ_CONSTANT();
            Element ar = POP();

            if (ar.type != CLOSURE)
            {
                WRITE_AR(el.arena, ar.arena);
            }
            else
            {
                ar.closure->func->name = el.arena;
                CPUSH(ar);
            }
        }
        break;
        case OP_GET_GLOBAL:
        {
            Arena ar = READ_CONSTANT().arena;
            Closure *c = GET_FUNC(ar);

            if (c)
                PUSH(CLOSURE(c));
            else
                PUSH(OBJ(find(ar)));
        }
        break;
        case OP_GET_CLOSURE:
            PUSH((machine.call_stack + READ_BYTE())->as);
            break;
        case OP_GET_NATIVE:
            PUSH(GET_NATIVE(READ_CONSTANT().arena));
            break;
        case OP_NOOP:
            break;
        case OP_GLOBAL_DEF:
        {
            Element el = READ_CONSTANT();
            Element res = POP();

            if (res.type != CLOSURE)
            {
                WRITE_AR(el.arena, res.arena);
            }
            else
            {
                res.closure->func->name = el.arena;
                CPUSH(res);
            }
        }
        break;
        case OP_PRINT:
            print(POP());
            break;
        case OP_RETURN:
        {

            Element el = POP();
            --machine.frame_count;

            if (machine.frame_count == 0)
            {
                POP();
                return INTERPRET_SUCCESS;
            }
            for (Stack *s = machine.stack; s < machine.stack->top; s++)
                POP();

            machine.stack->top = frame->slots;
            PUSH(el);

            frame = &machine.frames[machine.frame_count - 1];
            break;
        }
        }
    }
#undef CPOP
#undef POP
#undef WRITE_FUNC
#undef WRITE_AR
#undef FIND_NATIVE
#undef FIND_FUNC
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
