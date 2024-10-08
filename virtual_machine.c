#include "compiler.h"
#include "error.h"
#include "native.h"
#include "vector.h"
#include "virtual_machine.h"
#include "vm_util.h"

#define COUNT() (machine.stack.main->count)

#define IFIELD_COUNT() (machine.stack.init_field->count)
#define IFIELD()       (machine.stack.init_field->fields + IFIELD_COUNT() - 1)

void initVM(void)
{

	initialize_global_mem();

	machine.stack.main       = NULL;
	machine.stack.obj        = NULL;
	machine.stack.init_field = NULL;

	machine.glob = NULL;
	// machine.init_fields = NULL;
	machine.open_upvals = NULL;
	machine.caller      = NULL;
	machine.repl_native = NULL;

	machine.stack.main = GROW_STACK(NULL, STACK_SIZE);
	machine.glob       = GROW_TABLE(NULL, STACK_SIZE);

	machine.count.argc   = 0;
	machine.count.frame  = 0;
	machine.count.cargc  = 0;
	machine.count.native = 0;
}
void freeVM(void)
{
	if (machine.repl_native)
	{
		FREE(machine.repl_native->records);
		FREE(machine.repl_native);
	}

	// FREE_TABLE(&machine.repl_native);
	FREE_TABLE(&machine.glob);
	FREE_STACK(&machine.stack.main);
	FREE_STACK(&machine.stack.obj);
	free_field_stack(&machine.stack.init_field);

	machine.glob        = NULL;
	machine.repl_native = NULL;
	machine.stack.main  = NULL;
	machine.stack.obj   = NULL;

#ifdef GLOBAL_MEM_ARENA
	destroy_global_memory();
#endif
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
		function  *func  = frame->closure->func;
		int        line  = frame->closure->func->ch.lines[i];

		if (!func->name.val)
			fprintf(stderr, "script\n");
		else
			fprintf(stderr, "%s()\n", func->name.val);
		fprintf(stderr, "[line %d] in script\n", line);
	}

	machine.count.frame = 0;
}

static bool call(closure *c, uint8_t argc)
{

	if (c->func->arity != argc)
	{
		runtime_error(
		    "ERROR: Expected `%d` args, but got `%d`.", c->func->arity, argc
		);
		return false;
	}

	if (machine.count.frame == FRAMES_MAX)
	{
		runtime_error("ERROR: stack overflow.");
		return false;
	}

	CallFrame *frame = machine.frames + machine.count.frame++;

	frame->closure   = NULL;
	frame->ip        = NULL;
	frame->ip_return = NULL;
	frame->slots     = NULL;

	frame->closure      = c;
	frame->ip           = c->func->ch.ip;
	frame->return_index = machine.stack.main->count - (argc + 1);
	frame->slots        = (machine.stack.main->as + (COUNT() - (argc + 1)));
	return true;
}

void init_natives(void)
{
	machine.stack.obj = GROW_STACK(NULL, 3);
	define_natives(&machine.stack.obj);
	machine.repl_native = GROW_TABLE(NULL, INIT_SIZE);

	write_table(
	    machine.repl_native, Key("clock", 5),
	    NumType(machine.count.native++, T_NATIVE)
	);
	write_table(
	    machine.repl_native, Key("square", 6),
	    NumType(machine.count.native++, T_NATIVE)
	);
	write_table(
	    machine.repl_native, Key("file", 4),
	    NumType(machine.count.native++, T_NATIVE)
	);
}

Interpretation interpret(const char *src)
{

	function *func = NULL;

	if (!(func = compile(src, &machine.repl_native)))
		return INTERPRET_RUNTIME_ERR;

	closure *clos = _closure(func);
	call(clos, 0);
	machine.frames[machine.count.frame - 1].slots = machine.stack.main->as;

	machine.stack.obj =
	    GROW_STACK(&machine.stack.obj, machine.count.native + func->objc);

	push(&machine.stack.main, GEN(clos, T_CLOSURE));

	close_upvalues();
	return run();
}

Interpretation
interpret_path(const char *src, const char *path, const char *name)
{

	function *func = NULL;

	if (!(func = compile_path(src, path, name)))
		return INTERPRET_RUNTIME_ERR;

	closure *clos = _closure(func);
	call(clos, 0);
	machine.frames[machine.count.frame - 1].slots = machine.stack.main->as;

	machine.stack.obj = GROW_STACK(NULL, func->objc);

	define_natives(&machine.stack.obj);
	push(&machine.stack.main, GEN(clos, T_CLOSURE));

	close_upvalues();
	return run();
}

static uint8_t ifield_init(void)
{
	if (IFIELD_COUNT() > 0)
		return IFIELD()->init;
	return 0;
}

static bool call_value(element el, uint8_t argc)
{
	switch (el.type)
	{

	case T_CLOSURE:
		return call(CLOSURE(el), argc);
	case T_NATIVE:
	{
		element res =
		    NATIVE(el)->fn(argc, machine.stack.main->as + (COUNT() - argc));
		machine.stack.main->count -= (argc + 1);
		push(&machine.stack.main, res);
		return true;
	}

	default:
		break;
	}
	runtime_error("ERROR: Can only call functions and classes.");
	return false;
}

static upval *capture_upvalue(element *closed, uint8_t index)
{
	upval *prev = NULL;
	upval *curr = machine.open_upvals;

	for (; curr && curr->index > index; curr = curr->next)
		prev = curr;

	if (curr && curr->index == index)
		return curr;

	upval *new = NULL;
	new        = _upval(*(closed + index), index);
	new->next  = curr;

	if (prev)
		prev->next = new;
	else
		machine.open_upvals = new;
	return new;
}

static void close_upvalues(void)
{
	uint8_t local = COUNT() - 1;
	for (; machine.open_upvals && machine.open_upvals->index >= local;
	     machine.open_upvals = machine.open_upvals->next)
		;
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

	CallFrame *frame = machine.frames + (machine.count.frame - 1);

	register uint8_t *ip     = frame->ip;
	register uint8_t *ip_tmp = NULL;
	uint16_t          offset = 0;
	uint8_t           argc   = 0;
	element           obj;
	_key              key;

#define READ_BYTE() (*ip++)
#define READ_CONSTANT()                                                        \
	(*(frame->closure->func->ch.constants->as + READ_BYTE()))

#define UPPER() ((READ_BYTE() << 8) & 0xFF)
#define LOWER() (READ_BYTE() & 0xFF)

#define POP()    (pop(&machine.stack.main))
#define POPN(n)  (popn(&machine.stack.main, n))
#define PUSH(ar) (push(&machine.stack.main, ar))
#define PEEK()   (*(machine.stack.main->as + (COUNT() - 1)))
#define NPEEK(N) (*(machine.stack.main->as + (COUNT() - 1 - N)))

#define UPVALUE() ((*(frame->closure->upvals + READ_BYTE()))->closed)

#define FALSEY() (!POP()->val.Bool)
#define TRUTHY() (POP()->val.Bool)

#define ITAB() (pop_itab(&machine.stack.init_field))

#define LOCAL()   (*(frame->slots + READ_BYTE()))
#define NLOCAL(n) (*(frame->slots + n))

#define OBJECT()    (*(machine.stack.obj->as + READ_BYTE()))
#define NOB_JECT(n) (*(machine.stack.obj->as + n))

#define SET_OBJ(n, el) ((NOB_JECT(n) = el))

#define GET(ar)   (find_entry(&machine.glob, &ar))
#define SET(a, b) (write_table(machine.glob, a, b))

	for (;;)
	{
#ifdef DEBUG_TRACE_EXECUTION
		for (element *e = frame->slots;
		     e < machine.stack.main->as + COUNT(); e++)
			print(*e);
		disassemble_instruction(
		    &frame->closure->func->ch, (int)(ip - frame->ip)
		);
#endif

		switch (READ_BYTE())
		{
		case OP_CONSTANT:
			PUSH(READ_CONSTANT());
			break;
		case OP_CLOSURE:
		{
			obj = READ_CONSTANT();

			SET_OBJ(READ_BYTE(), obj);

			for (int i = 0; i < CLOSURE(obj)->uargc; i++)
				CLOSURE(obj)->upvals[i] =
				    (READ_BYTE())
					  ? capture_upvalue(frame->slots, READ_BYTE())
					  : frame->closure->upvals[READ_BYTE()];
		}
		break;

		case OP_METHOD:
		{
			obj = READ_CONSTANT();

			for (int i = 0; i < CLOSURE(obj)->uargc; i++)
				CLOSURE(obj)->upvals[i] =
				    (READ_BYTE())
					  ? capture_upvalue(frame->slots, READ_BYTE())
					  : frame->closure->upvals[READ_BYTE()];
		}
		break;

		case OP_GET_UPVALUE:
			PUSH(UPVALUE());
			break;
		case OP_SET_UPVALUE:
			UPVALUE() = PEEK();
			break;
		case OP_CLOSE_UPVAL:
			break;
		case OP_NEG:
			*(machine.stack.main->as + --COUNT()) =
			    _neg((machine.stack.main->as + COUNT()++));
			break;
		case OP_INC:
			*(machine.stack.main->as + --COUNT()) =
			    _inc((machine.stack.main->as + COUNT()++));
			break;
		case OP_DEC:
			*(machine.stack.main->as + --COUNT()) =
			    _dec((machine.stack.main->as + COUNT()++));
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
		case OP_CAST:

			PUSH(_cast(POP(), READ_BYTE()));
			break;
		case OP_TO_STR:
			if (PEEK().type != T_STR)
				PUSH(_to_str(POP()));
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
			machine.count.argc  = 0;
			break;
		case OP_NOOP:
			PUSH(Null());
			break;

		case OP_CLASS:
		{
			class *c = CLASS(OBJECT());
			push_itab(
			    &machine.stack.init_field, 1, copy_table(c->closures)
			);
			PUSH(GEN(c->init, T_CLOSURE));
			break;
		}
		case OP_ALLOC_INSTANCE:
		{

			instance *inst = NULL;
			inst           = _instance(CLASS(OBJECT()));
			uint8_t init   = READ_BYTE();

			inst->fields = (init) ? ITAB().fields
			                      : copy_table(inst->classc->closures);

			PUSH(GEN(inst, T_INSTANCE));
			break;
		}
		case OP_THIS:
			if (machine.caller != NULL)
				PUSH(GEN(machine.caller, T_INSTANCE));
			break;
		case OP_SET_PROP:
		{

			obj            = *POP();
			element inst   = *POP();
			uint8_t ifield = ifield_init();

			if (inst.type != T_INSTANCE && !ifield)
			{
				runtime_error(
				    "ERROR: Can only set properties of an instance."
				);
				return INTERPRET_RUNTIME_ERR;
			}

			write_table(
			    ifield ? IFIELD()->fields : INSTANCE(inst)->fields,
			    READ_CONSTANT().key, obj
			);
			PUSH(obj);
		}
		break;
		case OP_GET_PROP:
		{

			element inst   = *POP();
			uint8_t ifield = ifield_init();

			if (inst.type != T_INSTANCE && !ifield)
			{
				runtime_error(
				    "ERROR: Only instances contain properties."
				);
				return INTERPRET_RUNTIME_ERR;
			}

			key = READ_CONSTANT().key;

			if (ifield)
				obj = find_entry(&IFIELD()->fields, &key);
			else
			{
				obj = find_entry(&INSTANCE(inst)->fields, &key);
				machine.caller = INSTANCE(inst);
			}

			if (obj.type != T_NULL)
			{
				PUSH(obj);
				break;
			}
			runtime_error("ERROR: Undefined property '%s'.", key.val);
			return INTERPRET_RUNTIME_ERR;
		}
		case OP_SET_ACCESS:
		{
			if (NPEEK(1).type != T_NUM)
			{
				runtime_error(
				    "Attempting to access array with invalid type"
				);
				return INTERPRET_RUNTIME_ERR;
			}
			element *el   = NULL;
			element *vect = NULL;
			el            = POP();
			obj           = *POP();
			vect          = POP();

			_set_index((Long)obj.val.Num, el, &vect);
			break;
		}
		case OP_GET_ACCESS:
			if ((obj = PEEK()).type != T_NUM)
			{
				runtime_error(
				    "Attempting to access array with invalid type"
				);
				return INTERPRET_RUNTIME_ERR;
			}
			if ((obj = _get_index(POP()->val.Num, POP())).type == T_NULL)
			{
				runtime_error("Invalid array access");
				return INTERPRET_RUNTIME_ERR;
			}
			PUSH(obj);
			break;
		case OP_CALL:
		{
			uint8_t argc       = READ_BYTE();
			uint8_t is_closure = 0;

			if (NPEEK(argc).type == T_CLOSURE)
			{
				ip_tmp     = ip;
				is_closure = 1;
			}

			if (!call_value(NPEEK(argc), argc))
				return INTERPRET_RUNTIME_ERR;

			frame = (machine.frames + (machine.count.frame - 1));

			if (is_closure)
			{
				ip               = frame->ip;
				frame->ip_return = ip_tmp;
				ip_tmp           = NULL;
			}

			machine.count.argc  = argc;
			machine.count.cargc = 1;
			break;
		}
		case OP_INSTANCE:
			machine.count.argc  = READ_BYTE();
			machine.count.cargc = 1;
			break;
		case OP_JMPF:
			offset = UPPER(), offset = (offset | LOWER()) * FALSEY();
			ip += offset;
			break;
		case OP_JMPT:
			offset = UPPER(), offset = (offset | LOWER()) * TRUTHY();
			ip += offset;
			break;
		case OP_JMPL:
			offset = UPPER(), offset |= LOWER();
			ip     = frame->ip +
			     *(frame->closure->func->ch.cases.bytes + offset);
			break;
		case OP_JMP_NIL:
			offset = UPPER(), offset |= LOWER();
			if (null(PEEK()))
				ip += offset;
			break;
		case OP_JMP_NOT_NIL:
			offset = UPPER(), offset |= LOWER();
			if (not_null(PEEK()))
				ip += offset;
			break;
		case OP_JMP:
			offset = UPPER(), offset |= LOWER();
			ip += offset;
			break;
		case OP_LOOP:
			offset = UPPER(), offset |= LOWER();
			ip -= offset;
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
			argc = READ_BYTE();
			SET_OBJ(argc, READ_CONSTANT());
			break;
		case OP_LEN:
			PUSH(_len(POP()));
			break;
		case OP_RM:
			FREE_OBJ(*POP());
			break;
		case OP_DELETE_VAL:
		{

			int      index = POP()->val.Num;
			element *v     = NULL;
			v              = POP();
			delete_index(&v, index);
			break;
		}
		case OP_INSERT_VAL:
		{

			element *o     = POP();
			int      index = POP()->val.Num;
			// v              = VECTOR((*POP()));
			element *vect = POP();
			_insert(&vect, o, (Long)index);
			break;
		}
		case OP_PUSH_VAL:
		{
			element *o    = POP();
			element *vect = POP();
			push_obj(&vect, o);
			break;
		}
		case OP_POP_VAL:
		{
			element *vect = POP();
			pop_obj(&vect);
			break;
		}
		case OP_ALLOC_TABLE:
			if (PEEK().type != T_NUM)
			{
				runtime_error("ERROR: table argument must be "
				              "a numeric value.");
				return INTERPRET_RUNTIME_ERR;
			}
			PUSH(GEN(GROW_TABLE(NULL, POP()->val.Num), T_TABLE));
			break;
		case OP_GET_GLOBAL:
			key = READ_CONSTANT().key;
			obj = GET(key);

			if (obj.type != T_NULL)
			{
				PUSH(obj);
				break;
			}

			runtime_error("ERROR: Undefined property '%s'.", key.val);
			return INTERPRET_RUNTIME_ERR;
		case OP_GLOBAL_DEF:
			key = READ_CONSTANT().key;
			obj = *POP();

			if (GET(key).type != T_NULL)
			{
				error(
				    "Duplicate global variable "
				    "identifier: %s\n",
				    key.val
				);
				return INTERPRET_RUNTIME_ERR;
			}

			SET(key, obj);
			break;
		case OP_SET_GLOBAL:
			SET(READ_CONSTANT().key, *POP());
			break;
		case OP_SET_FUNC_VAR:
			key = READ_CONSTANT().key;
			obj = (machine.count.cargc < machine.count.argc)
			          ? *(frame->slots + machine.count.cargc++)
			          : *POP();

			SET(key, obj);
			break;
		case OP_PRINT:
			print(*POP());
			break;
		case OP_RETURN:
			obj = *POP();
			--machine.count.frame;

			if (machine.count.frame == 0)
				return INTERPRET_SUCCESS;

			machine.stack.main->count = frame->return_index;
			PUSH(obj);

			ip    = frame->ip_return;
			frame = &machine.frames[machine.count.frame - 1];
			break;
		}
	}
#undef READ_BYTE
#undef UPPER
#undef LOWER
#undef READ_SHORT
#undef READ_CONSTANT
#undef POP
#undef POPN
#undef PUSH
#undef PEEK
#undef NPEEK
#undef FALSEY
#undef TRUTHY
#undef LOCAL
#undef NLOCAL
#undef OBJECT
#undef GET
#undef SET
}
