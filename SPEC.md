Project: Ykes — Language Specification (Draft)

Goals
- Provide a small, embeddable programming language and VM focused on clarity and extensibility.
- Prioritize safety, predictable semantics, and good developer ergonomics.
- Target: Linux (x86_64, arm64), macOS (x86_64, arm64), FreeBSD where feasible.
- Early major versions (0.x or 1.0) may introduce breaking changes; compatibility guarantees will be tightened in later releases.
- License: Apache License 2.0.

Priority features for initial production push
- Module system: deterministic import semantics, package namespace isolation, and simple module resolution.
- FFI: well-documented C FFI for native calls and embedding the VM.
- Standard library I/O: file, console, basic networking primitives as safe wrappers.
- Concurrency: lightweight concurrency primitives (green threads / coroutines) and safe synchronization primitives.

High-level language design
- Syntax: concise, expression-first, C-like control constructs with first-class functions and closures.
- Types: dynamically typed values at runtime; consider optional gradual typing in a later phase.
- Memory model: currently manual allocator / GC hybrid; aim to harden via tests and sanitizers, then evaluate a compact precise GC.

Module system (initial design)
- Module files end with `.yk` and are imported by path or package name.
- `import "mod"` resolves in the following order: local relative path, project package path, system package path.
- Modules expose an explicit `export` list; otherwise top-level bindings are local to the module.

FFI (initial design)
- Provide a stable C ABI boundary: `yk_value` and helpers for conversion to/from C types.
- Loadable native libraries via `native` registration; a `ffi` module will expose helpers to open libraries and bind symbols safely.

Standard library (initial modules)
- `io`: `read`, `write`, `append`, atomic file helpers.
- `net`: simple TCP client/server (non-blocking) — optional in initial release.
- `math`, `time`, `concurrency`, `table`, `vector`.

Concurrency (initial design)
- Provide coroutines (green threads) and a scheduler integrated with the VM.
- Provide channels or message-passing primitives as the primary synchronization mechanism.

Testing & quality
- Canonical test-suite lives in `tests/` with inputs and expected outputs.
- CI will run builds on Ubuntu/macOS, ASan/UBSan on Linux, and run valgrind in Docker for leak detection.

Roadmap (short-term)
1. Stabilize build and CI (ASan + Valgrind) and fix all memory issues.
2. Draft `SPEC.md` and canonical tests; add test harness.
3. Implement module import semantics and FFI helpers.
4. Add standard library core modules and concurrency primitives.

Acceptance criteria for production-ready baseline
- Cross-platform builds on Linux/macOS.
- CI with ASan/UBSan and nightly Valgrind runs showing no new regressions.
- Module system and minimal standard library (`io`, `math`, `time`, `concurrency`) implemented and tested.
- Documentation: `SPEC.md`, README, and quickstart guide.

Notes
- This is a draft spec to be iterated. Implementation may reveal design constraints requiring spec changes; tests drive spec stabilization.

Lexical grammar (informal)
- Identifiers: [A-Za-z_][A-Za-z0-9_]*
- Numbers: decimal floating point or integer literals (e.g. 123, 3.14)
- Strings: double-quoted and single-quoted strings with C-like escapes
- Comments: `//` single-line and `/* ... */` block comments

Syntax (core)
- Expression-oriented language. Examples:

	- Variable binding:

		let x = 1;

	- Function definition:

		fn add(a, b) {
			return a + b;
		}

	- If / else:

		if (x > 0) {
			print("positive");
		} else {
			print("non-positive");
		}

	- Loop:

		for (i = 0; i < 10; i = i + 1) {
			print(i);
		}

	- Coroutines (sketch):

		co fib(n) {
			if (n < 2) return n;
			yield fib(n-1) + fib(n-2);
		}

Module system (detailed sketch)
- Import forms:
	- `import "./path/to/mod.yk" as m;`  // relative path
	- `import "pkg:name" as pkg;`         // package resolution
- Resolution order:
	1. If path is relative (starts with `.`), resolve against current file directory.
	2. If not found, search project package paths (`YK_MODULE_PATH` or repo root `lib/`).
	3. If not found and system-installed package paths exist, search there.
- Exports:
	- `export foo, Bar` at module top-level exposes names to importers.
	- By default, top-level names are private to the module unless exported.

FFI (C) interface sketch
- Provide helpers in `includes/yk_ffi.h` with two main value representations:
	- `yk_value` (opaque runtime value)
	- Conversion helpers: `yk_to_int(yk_value)`, `yk_from_int(int)` etc.
- Bind a native function by registering a C function pointer via `register_native("name", fn_ptr)` during initialization.
- Safe calling conventions: argument count checked; return `yk_value`.

Concurrency model (sketch)
- Provide lightweight coroutines managed by VM scheduler.
- Communicate via message-passing channels; channels can be buffered or unbuffered.
- Blocking operations yield to the scheduler; the scheduler runs coroutines cooperatively.

Error model
- Runtime errors are reported with source file/line info when available.
- The VM aims to avoid undefined behavior; invalid memory accesses should be caught by sanitizers and fixed.

Testing strategy
- Tests are canonicalized in `tests/expected/*.out` for integration outputs.
- Unit tests for core components (scanner, parser, vm) live under `tests/unit/` and are run in CI.

Stability policy
- Early major versions (0.x / 1.0) may include breaking changes; we will provide migration notes in changelogs and upgrade tooling.

Extensibility notes
- The specification is intentionally pragmatic — concrete decisions may be adjusted as implementation and tests reveal trade-offs.

