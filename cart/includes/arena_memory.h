#ifndef _ARENA_MEMORY_H
#define _ARENA_MEMORY_H

#include <string.h>
#include "arena.h"
#include "common.h"

#define LOAD_FACTOR 0.75
#define FRAMES_MAX 500
#define TAKE_OUT_THE_TRASH 250
#define CAPACITY 50
#define INC 2
#define PAGE 16384
#define PAGE_COUNT 16
#define INIT_GLOBAL \
    (PAGE * PAGE_COUNT)
#define STACK_SIZE 50
#define NATIVE_STACK_SIZE 25
#define TABLE_SIZE 50
#define IP_SIZE 100
#define MEM_OFFSET 1

#define ALLOC(size) \
    alloc_ptr(size + OFFSET)

#define PTR(ptr) \
    ((Free *)ptr - OFFSET)

#define DPTR(ptr) \
    ((Free **)ptr)

#define FREE(ptr) \
    free_ptr(PTR(ptr))

#define GROW_CAPACITY(capacity) \
    ((capacity) < CAPACITY ? CAPACITY : capacity * INC)

#define GROW_ARRAY(ar, size, type) \
    arena_realloc(ar, size, type)
#define GROW_ARENA(ar, size) \
    arena_realloc_arena(ar, size)
#define FREE_ARENA(ar) \
    arena_realloc_arena(ar, 0)
#define FREE_ARRAY(ar) \
    arena_realloc(ar, 0, ARENA_NULL)
#define ARENA_FREE(ar) \
    arena_free(ar)

#define GROW_TABLE(ar, size) \
    arena_realloc_table(ar, size)
#define ALLOC_ENTRY(a, b) \
    alloc_entry(a, b)
#define FREE_TABLE(ar) \
    arena_realloc_table(ar, 0)
#define FREE_TABLE_ENTRY(ar) \
    arena_free_entry(ar)

#define GROW_STACK(st, size) \
    realloc_stack(st, size)
#define FREE_STACK(st) \
    free_stack(st)

#define FREE_FUNCTION(func) \
    free_function(func)
#define FREE_NATIVE(nat) \
    free_native(nat)
#define FREE_CLOSURE(clos) \
    free_closure(clos)
#define FREE_UPVALS(up) \
    free_upvals(up)

#define FREE_UPVAL(up) \
    free_upval(up)
#define NEW_STACK(size) \
    stack(size)

#define OBJ(o) \
    Obj(o)
#define FUNC(ar) \
    Func(ar)
#define NATIVE(n) \
    native_fn(n)
#define CLOSURE(c) \
    closure(c)
#define UPVAL(c) \
    upval_el(c)
#define CLASS(c) \
    new_class(class(c))
#define FREE_CLASS(c) \
    free_class(c)

typedef union Free Free;
typedef struct Garbage Garbage;
typedef long long int Align;

union Free
{

    struct
    {
        size_t size;
        Free *next;
    };
    Align align;
};

struct Garbage
{
    Free *obj;
};

struct CallFrame
{
    Closure *closure;
    uint8_t *ip;
    uint8_t *ip_start;
    Stack *slots;
};
typedef struct CallFrame CallFrame;

struct vm
{
    int frame_count;
    int garbage_count;
    int garbage_len;
    CallFrame frames[FRAMES_MAX];
    Stack *stack;
    Stack *call_stack;
    Stack *native_calls;
    Upval *open_upvals;
    Table *glob;
    Garbage *garbage;
};
typedef struct vm vm;
typedef vm *Vm;

vm machine;
static Free *mem;

void initialize_global_memory();
void destroy_global_memory();

Arena *arena_alloc_arena(size_t size);
Arena *arena_realloc_arena(Arena *ar, size_t size);
void arena_free_arena(Arena *ar);

void *alloc_ptr(size_t size);
void free_ptr(Free *new);
void free_garbage(Free **new);

Arena arena_init(void *data, size_t size, T type);
Arena arena_alloc(size_t size, T type);
Arena arena_realloc(Arena *ar, size_t size, T type);

Arena Char(char ch);
Arena Int(int ival);
Arena Byte(uint8_t byte);
Arena Long(long long int llint);
Arena Double(double dval);
Arena String(const char *str);
Arena CString(const char *str);
Arena Bool(bool boolean);
Arena Size(size_t Size);
Arena Null();

void arena_free(Arena *ar);

void init_chunk(Chunk *c);
void free_chunk(Chunk *c);

Stack *stack(size_t size);
Stack *realloc_stack(Stack *stack, size_t size);
void free_stack(Stack **stack);

Upval **upvals(size_t size);
void free_upvals(Upval **up);

Stack value(Element el);
Element Obj(Arena ar);
Element Func(Function *f);
Element upval_el(Upval *up);
Element native_fn(Native *native);
Element closure(Closure *clos);
Element new_class(Class *classc);
Element null_obj();

Function *function(Arena name);
void free_function(Function *func);

Closure *new_closure(Function *func);
void free_closure(Closure *closure);

Upval *upval(Stack *index);
void free_upval(Upval *up);

Native *native(NativeFn native, Arena ar);
void free_native(Native *native);

void print(Element ar);

Table Entry(Arena key, Element val);
Table arena_entry(Arena key, Arena val);
Table class_entry(Class *c);
Table func_entry(Closure *c);
Table native_entry(Native *func);
Table new_entry(Table t);
size_t hash(Arena key);
Table *arena_alloc_table(size_t size);
Table *arena_realloc_table(Table *t, size_t size);

void alloc_entry(Table **e, Table el);
void arena_free_table(Table *t);
void arena_free_entry(Table *entry);

Arena Var(const char *str);
Arena func_name(const char *str);
Arena native_name(const char *str);

Class *class(Arena name);
void free_class(Class *c);

Instance *instance(Class *c);
void free_instance(Instance *ic);

#endif
