#include "includes/ffi.h"
#include "includes/object.h"
#include "includes/virtual_machine.h"
#include <stdio.h>

/* Simple native that adds two numbers. */
static element cadd(int argc, element *argv)
{
	double a = 0.0, b = 0.0;
	if (argc > 0 && argv[0].type == T_NUM)
		a = argv[0].val.Num;
	if (argc > 1 && argv[1].type == T_NUM)
		b = argv[1].val.Num;
	return Num(a + b);
}

int main(void)
{
	yk_vm_init();

	yk_register_native("cadd", cadd);

	/* Run a small script that uses the registered native */
	const char *src = "print(cadd(2, 3));";

	interpret(src);

	yk_vm_free();
	return 0;
}
