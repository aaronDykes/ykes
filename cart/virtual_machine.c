#include "compiler.h"
#include "arena_math.h"
#include "virtual_machine.h"
#include <stdarg.h>

static void check_stack_size();
static void reset_stack();
static vm machine;

void initVM()
{

    initialize_global_memory(PAGE * 16);
    machine.max_size = STACK_SIZE;
    machine.current_size = 0;
    machine.stack = GROW_ARENA(NULL, sizeof(arena) * STACK_SIZE);
    init_dict(&machine.d);
    reset_stack();
}
void freeVM()
{
    free_dict(&machine.d);
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
        machine.stack = GROW_ARENA(machine.stack, sizeof(arena) * machine.max_size);
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
    if (n.type == ARENA_VAR)
        ar = find_entry(&machine.d.map, &n);
    switch (ar.type)
    {
    case ARENA_DOUBLE_CONST:
        ar.as.dval = -ar.as.dval;
        break;
    case ARENA_INT_CONST:
        ar.as.ival = -ar.as.ival;
        break;
    case ARENA_LLINT_CONST:
        ar.as.llint = -ar.as.llint;
        break;
    case ARENA_BOOL:
        ar.as.boolean = !ar.as.boolean;
        break;
    case ARENA_NULL:
        return arena_bool(true);
    default:
        runtime_error("ERROR: negation type mismatch");
    }
    return ar;
}
static arena _add(arena a, arena b)
{

    arena ar1 = a, ar2 = b;
    if (a.type == ARENA_VAR)
        ar1 = find_entry(&machine.d.map, &a);
    else if (b.type == ARENA_VAR)
        ar2 = find_entry(&machine.d.map, &b);

    switch (ar2.type)
    {
    case ARENA_CHAR_CONST:
        return add_arena_char(ar2.as.ch, ar1);
    case ARENA_DOUBLE_CONST:
        return add_arena_double(ar2.as.dval, ar1);
    case ARENA_INT_CONST:
        return add_arena_int(ar2.as.ival, ar1);
    case ARENA_LLINT_CONST:
        return add_arena_llint(ar2.as.llint, ar1);
    case ARENA_STR:
        return append(ar2, ar1);
    }
    return ar1;
}

static arena _sub(arena a, arena b)
{
    arena ar1 = a, ar2 = b;
    if (a.type == ARENA_VAR)
        ar1 = find_entry(&machine.d.map, &a);
    else if (b.type == ARENA_VAR)
        ar2 = find_entry(&machine.d.map, &b);

    switch (ar2.type)
    {
    case ARENA_DOUBLE_CONST:
        return sub_arena_double(ar2.as.dval, ar1);
    case ARENA_INT_CONST:
        return sub_arena_int(ar2.as.ival, ar1);
    case ARENA_CHAR_CONST:
        return sub_arena_char(ar2.as.ch, ar1);
    case ARENA_LLINT_CONST:
        return sub_arena_llint(ar2.as.llint, ar1);
    }
    return ar2;
}

static arena _mul(arena a, arena b)
{
    arena ar1 = a, ar2 = b;
    if (a.type == ARENA_VAR)
        ar1 = find_entry(&machine.d.map, &a);
    else if (b.type == ARENA_VAR)
        ar2 = find_entry(&machine.d.map, &b);

    switch (ar1.type)
    {
    case ARENA_DOUBLE_CONST:
        return mul_arena_double(ar1.as.dval, ar2);
    case ARENA_CHAR_CONST:
        return mul_arena_char(ar1.as.ch, ar2);
    case ARENA_INT_CONST:
        return mul_arena_int(ar1.as.ival, ar2);
    case ARENA_LLINT_CONST:
        return mul_arena_llint(ar1.as.llint, ar2);
    }
    return ar1;
}

static arena _div(arena a, arena b)
{
    arena ar1 = a, ar2 = b;
    if (a.type == ARENA_VAR)
        ar1 = find_entry(&machine.d.map, &a);
    else if (b.type == ARENA_VAR)
        ar2 = find_entry(&machine.d.map, &b);

    switch (ar2.type)
    {
    case ARENA_CHAR_CONST:
        return div_arena_char(ar2.as.ch, ar1);
    case ARENA_DOUBLE_CONST:
        return div_arena_double(ar2.as.dval, ar1);
    case ARENA_INT_CONST:
        return div_arena_int(ar2.as.ival, ar1);
    case ARENA_LLINT_CONST:
        return div_arena_llint(ar2.as.llint, ar1);
    }
    return ar2;
}

static arena _mod(arena a, arena b)
{
    arena ar1 = a, ar2 = b;
    if (a.type == ARENA_VAR)
        ar1 = find_entry(&machine.d.map, &a);
    else if (b.type == ARENA_VAR)
        ar2 = find_entry(&machine.d.map, &b);

    switch (ar2.type)
    {
    case ARENA_CHAR_CONST:
        return mod_arena_char(ar2.as.ch, ar1);
    case ARENA_INT_CONST:
        return mod_arena_int(ar2.as.ival, ar1);
    case ARENA_LLINT_CONST:
        return mod_arena_llint(ar2.as.llint, ar1);
    }

    return ar2;
}

static arena _eq(arena a, arena b)
{
    arena ar1 = a, ar2 = b;
    if (a.type == ARENA_VAR)
        ar1 = find_entry(&machine.d.map, &a);
    else if (b.type == ARENA_VAR)
        ar2 = find_entry(&machine.d.map, &b);

    switch (ar2.type)
    {
    case ARENA_INT_CONST:
        return int_eq(ar2.as.ival, ar1);
    case ARENA_DOUBLE_CONST:
        return double_eq(ar2.as.dval, ar1);
    case ARENA_LLINT_CONST:
        return llint_eq(ar2.as.llint, ar1);
    case ARENA_CHAR_CONST:
        return char_eq(ar2.as.ch, ar1);
    case ARENA_STR:
        return string_eq(ar2, ar1);
    case ARENA_NULL:
        switch (ar1.type)
        {
        case ARENA_BOOL:
            return arena_bool(ar1.as.boolean ? false : true);
        case ARENA_NULL:
            return arena_bool(ar1.as.boolean == ar2.as.boolean);
        default:
            runtime_error("ERROR: Comparison type mismatch\n");
        }
    case ARENA_BOOL:
        switch (ar1.type)
        {
        case ARENA_BOOL:
            return arena_bool(ar1.as.boolean == ar2.as.boolean);
        case ARENA_NULL:
            return arena_bool(ar2.as.boolean ? false : true);
        default:
            runtime_error("ERROR: Comparison type mismatch\n");
        }
    }
    return ar2;
}
static arena _ne(arena a, arena b)
{
    arena ar1 = a, ar2 = b;
    if (a.type == ARENA_VAR)
        ar1 = find_entry(&machine.d.map, &a);
    else if (b.type == ARENA_VAR)
        ar2 = find_entry(&machine.d.map, &b);

    switch (ar2.type)
    {
    case ARENA_INT_CONST:
        return int_ne(ar2.as.ival, ar1);
    case ARENA_DOUBLE_CONST:
        return double_ne(ar2.as.dval, ar1);
    case ARENA_LLINT_CONST:
        return llint_ne(ar2.as.llint, ar1);
    case ARENA_CHAR_CONST:
        return char_ne(ar2.as.ch, ar1);
    case ARENA_STR:
        return string_ne(ar2, ar1);
    case ARENA_NULL:
        switch (ar1.type)
        {
        case ARENA_BOOL:
            return arena_bool(ar1.as.boolean ? true : false);
        case ARENA_NULL:
            return arena_bool(ar1.as.boolean != ar2.as.boolean);
        default:
            runtime_error("ERROR: Comparison type mismatch\n");
        }
    case ARENA_BOOL:
        switch (ar1.type)
        {
        case ARENA_BOOL:
            return arena_bool(ar1.as.boolean != ar2.as.boolean);
        case ARENA_NULL:
            return arena_bool(ar2.as.boolean ? true : false);
        default:
            runtime_error("ERROR: Comparison type mismatch\n");
        }
    }
    return ar2;
}

static arena _lt(arena a, arena b)
{
    arena ar1 = a, ar2 = b;
    if (a.type == ARENA_VAR)
        ar1 = find_entry(&machine.d.map, &a);
    else if (b.type == ARENA_VAR)
        ar2 = find_entry(&machine.d.map, &b);

    switch (ar2.type)
    {
    case ARENA_INT_CONST:
        return int_lt(ar2.as.ival, ar1);
    case ARENA_DOUBLE_CONST:
        return double_lt(ar2.as.dval, ar1);
    case ARENA_LLINT_CONST:
        return llint_lt(ar2.as.llint, ar1);
    case ARENA_CHAR_CONST:
        return char_lt(ar2.as.ch, ar1);
    case ARENA_STR:
        return string_lt(ar2, ar1);
    }
    return ar2;
}
static arena _le(arena a, arena b)
{

    arena ar1 = a, ar2 = b;
    if (a.type == ARENA_VAR)
        ar1 = find_entry(&machine.d.map, &a);
    else if (b.type == ARENA_VAR)
        ar2 = find_entry(&machine.d.map, &b);

    switch (ar2.type)
    {
    case ARENA_INT_CONST:
        return int_le(ar2.as.ival, ar1);
    case ARENA_DOUBLE_CONST:
        return double_le(ar2.as.dval, ar1);
    case ARENA_LLINT_CONST:
        return llint_le(ar2.as.llint, ar1);
    case ARENA_CHAR_CONST:
        return char_le(ar2.as.ch, ar1);
    case ARENA_STR:
        return string_le(ar2, ar1);
    }
    return ar2;
}
static arena _gt(arena a, arena b)
{

    arena ar1 = a, ar2 = b;
    if (a.type == ARENA_VAR)
        ar1 = find_entry(&machine.d.map, &a);
    else if (b.type == ARENA_VAR)
        ar2 = find_entry(&machine.d.map, &b);

    switch (ar2.type)
    {
    case ARENA_INT_CONST:
        return int_gt(ar2.as.ival, ar1);
    case ARENA_DOUBLE_CONST:
        return double_gt(ar2.as.dval, ar1);
    case ARENA_LLINT_CONST:
        return llint_gt(ar2.as.llint, ar1);
    case ARENA_CHAR_CONST:
        return char_gt(ar2.as.ch, ar1);
    case ARENA_STR:
        return string_gt(ar2, ar1);
    }
    return ar2;
}
static arena _ge(arena a, arena b)
{
    arena ar1 = a, ar2 = b;
    if (a.type == ARENA_VAR)
        ar1 = find_entry(&machine.d.map, &a);
    else if (b.type == ARENA_VAR)
        ar2 = find_entry(&machine.d.map, &b);

    switch (ar2.type)
    {
    case ARENA_INT_CONST:
        return int_ge(ar2.as.ival, ar1);
    case ARENA_DOUBLE_CONST:
        return double_ge(ar2.as.dval, ar1);
    case ARENA_LLINT_CONST:
        return llint_ge(ar2.as.llint, ar1);
    case ARENA_CHAR_CONST:
        return char_ge(ar2.as.ch, ar1);
    case ARENA_STR:
        return string_ge(ar2, ar1);
    }
    return ar2;
}

static arena _or(arena a, arena b)
{
    arena ar1 = a, ar2 = b;
    if (a.type == ARENA_VAR)
        ar1 = find_entry(&machine.d.map, &a);
    else if (b.type == ARENA_VAR)
        ar2 = find_entry(&machine.d.map, &b);

    if (ar1.type != ARENA_BOOL || ar2.type != ARENA_BOOL)
    {
        runtime_error("ERROR: or type mismatch");
        return arena_bool(false);
    }
    return arena_bool(ar1.as.boolean || ar2.as.boolean);
}
static arena _and(arena a, arena b)
{
    arena ar1 = a, ar2 = b;
    if (a.type == ARENA_VAR)
        ar1 = find_entry(&machine.d.map, &a);
    else if (b.type == ARENA_VAR)
        ar2 = find_entry(&machine.d.map, &b);

    if (ar1.type != ARENA_BOOL || ar2.type != ARENA_BOOL)
    {
        runtime_error("ERROR: or type mismatch");
        return arena_bool(false);
    }
    return arena_bool(ar2.as.boolean && ar1.as.boolean);
}

static arena _assign(arena a, arena b)
{

    arena tmp;

    if (b.type != ARENA_VAR && a.type != ARENA_VAR)
        return b;

    if (a.type == ARENA_VAR)
    {
        tmp = find_entry(&machine.d.map, &a);

        if (tmp.type != ARENA_NULL)
            write_dict(&machine.d, &b, &tmp);
    }
    else
        write_dict(&machine.d, &b, &a);

    return find_entry(&machine.d.map, &b);
}

Interpretation run()
{

#define READ_BYTE() (*machine.ip.as.bytes++)
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
                                (int)(machine.ip.as.bytes - machine.ch->op_codes.as.bytes));
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
            push(arena_null());
            break;
        case OP_ASSIGN:
            push(_assign(POP(), POP()));
            break;
        case OP_NOOP:
            break;
        case OP_RETURN:
            tmp = POP();
            if (tmp.type == ARENA_VAR)
                print(find_entry(&machine.d.map, &tmp));
            else
                print(tmp);
            return INTERPRET_SUCCESS;
        }
    }

#undef POP
#undef READ_CONSTANT
#undef READ_BYTE
}
