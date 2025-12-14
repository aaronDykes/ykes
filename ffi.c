#include "includes/ffi.h"
#include "mem.h"
#include "object_memory.h"
#include "object_util.h"
#include "table.h"
#include "virtual_machine.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void yk_vm_init(void)
{
	initVM();
	init_natives();
}

void yk_vm_free(void)
{
	freeVM();
}

int yk_register_native(const char *name, NativeFn fn)
{
	if (!name || !fn)
		return -1;

	/* Ensure object native stack exists */
	if (!machine.stack.obj)
		machine.stack.obj = GROW_STACK(NULL, INIT_SIZE);

	/* Ensure there's space for the new native */
	machine.stack.obj =
	    GROW_STACK(&machine.stack.obj, machine.count.native + 1);

	/* Create key and native object */
	_key *k = Key(name, (int)strlen(name));

	element el = GEN(_native(fn, k), T_NATIVE);
	*((machine.stack.obj)->as + machine.count.native) = el;

	/* Ensure repl_native table exists */
	if (!machine.repl_native)
		machine.repl_native = GROW_TABLE(NULL, INIT_SIZE);

	write_table(
	    machine.repl_native, k, NumType(machine.count.native, T_NATIVE)
	);

	return (int)(machine.count.native++);
}

static char *read_entire_file(const char *path)
{
	if (!path)
		return NULL;

	FILE *f = fopen(path, "rb");
	if (!f)
		return NULL;

	if (fseek(f, 0, SEEK_END) != 0)
	{
		fclose(f);
		return NULL;
	}
	long size = ftell(f);
	rewind(f);

	if (size < 0)
	{
		fclose(f);
		return NULL;
	}

	char *buf = ALLOC((size_t)size + 1);
	if (!buf)
	{
		fclose(f);
		return NULL;
	}

	size_t read = fread(buf, 1, (size_t)size, f);
	buf[read]   = '\0';
	fclose(f);
	return buf;
}

static void split_path(
    const char *fullpath, char *out_dir, size_t dirlen, char *out_name,
    size_t namelen
)
{
	/* simple dirname/basename helper */
	const char *p = strrchr(fullpath, '/');
	if (!p)
	{
		/* no directory part */
		if (out_dir && dirlen > 0)
			out_dir[0] = '\0';
		if (out_name && namelen > 0)
			strncpy(out_name, fullpath, namelen - 1),
			    out_name[namelen - 1] = '\0';
	}
	else
	{
		size_t dlen = (size_t)(p - fullpath + 1);
		if (out_dir)
		{
			strncpy(
			    out_dir, fullpath, (dlen < dirlen) ? dlen : dirlen - 1
			);
			out_dir[(dlen < dirlen) ? dlen : dirlen - 1] = '\0';
		}
		if (out_name)
		{
			strncpy(out_name, p + 1, namelen - 1);
			out_name[namelen - 1] = '\0';
		}
	}
}

int yk_load_module(const char *path, char **err_out)
{
	if (!path)
	{
		if (err_out)
		{
			*err_out = ALLOC(32);
			strcpy(*err_out, "Invalid path");
		}
		return 1;
	}

	char realbuf[PATH_MAX];
	if (!realpath(path, realbuf))
	{
		/* could not resolve path */
		return 1;
	}

	/* If module already loaded, nothing to do */
	_key *mod_key = Key(realbuf, (int)strlen(realbuf));

	if (machine.modules &&
	    find_entry(&machine.modules, mod_key).type != T_NULL)
		return 0;

	char dir[PATH_MAX];
	char name[PATH_MAX];
	split_path(realbuf, dir, sizeof(dir), name, sizeof(name));

	char *src = NULL;
	src       = read_entire_file(realbuf);

	if (!src)
		return 3;

	Interpretation r = interpret_path(src, dir, name);

	return (r != INTERPRET_SUCCESS) ? 1 : 0;
}
