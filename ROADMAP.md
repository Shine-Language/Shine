# Roadmap

## v0.1.0 - first steps
- [x] Lexer, parser, codegen for fn/return/call/literals
- [x] `write` builtin via `puts`
- [x] Object file emission + link, builds hello.shine
- [x] Function parameters, identifier expressions, per-function argument passing

## v0.2.0 - variables and operators
- [x] `let(type) name = expr;`
- [x] `var(type) name = expr;`
- [x] Assignment to `var`
- [x] `+ - * /`, comparisons

## v0.3.0 - control flow
- [x] if/else, while, for, etc.
    - while / for will be combined to `loop(condition) {}`
- [x] break (written as stop) and continue (written as cont) loop controls

## v0.4.0 - inputs & non string literals
- [ ] Add user inputs
    - Would be written as `user_input("TEXT")`
- [ ] Allow `write()` to support non string literals

## v0.5.0 - real types
- [ ] Type hierarchy instead of string-named TypeRef
- [ ] Pointers, fixed-width ints
- [ ] Type-checking pass (codegen currently does no validation of its own)

## Later
- [ ] Structs, arrays
- [ ] Standard library (move write out of the compiler)
- [ ] Multi-file modules
- [ ] C FFI (extern function declarations, calling into existing native
      libs).
- [ ] Package manager + package ecosystem (Cargo/pip-style) - a manifest
      format, dependency resolution, a CLI (install/publish/build), and
      eventually a package registry if others are meant to publish too.
      This is a large, separate project on top of the compiler itself;
      realistically only makes sense once Shine has multi-file modules
      (see above) and enough language stability that packages wouldn't
      immediately break across versions. Near-long term.
- [ ] Freestanding codegen mode (no libc, no CRT startup, custom entry
      point/linker script) - needed if Shine ever targets an OS/kernel.
      `write` would need a non-libc backend (e.g. direct UART/framebuffer)
      in that mode. Purely additive to the current hosted-executable path;
      not urgent while the language itself is this early.
