#ifndef _YK_FFI_H
#define _YK_FFI_H

#include "object_type.h"

/* Minimal embedding / FFI API for Ykes runtime */

/* Initialize and shutdown the VM */
void yk_vm_init(void);
void yk_vm_free(void);

/* Register a native C function available to Ykes programs.
 * Returns the native index (>=0) on success, -1 on error.
 */
int yk_register_native(const char *name, NativeFn fn);

/* Load and execute a Ykes source file from `path`.
 * On error, returns non-zero and may set `*err_out` to an error message
 * (caller must free with `FREE`). On success returns 0 and `*err_out` is
 * left untouched (may be NULL).
 */
int yk_load_module(const char *path, char **err_out);

/* Record an exported identifier while compiling/executing a module.
 * The compiler should call this when it encounters an `export` declaration
 * for a top-level identifier. The recorded exports are used by
 * `yk_load_module` to determine explicit module exports.
 */
void yk_record_export(_key *name);

#endif
