# Roadmap

## v0.1.0 - not yet published
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
- [ ] if/else, while, for

## v0.4.0 - real types
- [ ] Type hierarchy instead of string-named TypeRef
- [ ] Pointers, fixed-width ints
- [ ] Type-checking pass (codegen currently does no validation of its own)

## Later
- [ ] Structs, arrays
- [ ] Standard library (move write out of the compiler)
- [ ] Multi-file modules
- [ ] Freestanding codegen mode (no libc, no CRT startup, custom entry
      point/linker script) - needed if Shine ever targets an OS/kernel.
      `write` would need a non-libc backend (e.g. direct UART/framebuffer)
      in that mode. Purely additive to the current hosted-executable path;
      not urgent while the language itself is this early.
- [ ] C FFI (extern function declarations, calling into existing native
      libs) - this is the real path to UI capability. Rather than
      building a UI framework from scratch, let Shine call into mature
      native libraries (GTK, SDL, Win32, etc.) with minimal ceremony.
      Speed/light-weight already follow from native compilation + no
      runtime; FFI is what turns that into actually-usable UI work.
      Once FFI exists, revisit owning ONE UI lane directly (most likely
      "quick tool UIs" - settings panels, dashboards - rather than
      competing with Qt or a game engine head-on).
- [ ] Package manager + package ecosystem (Cargo/pip-style) - a manifest
      format, dependency resolution, a CLI (install/publish/build), and
      eventually a package registry if others are meant to publish too.
      This is a large, separate project on top of the compiler itself -
      realistically only makes sense once Shine has multi-file modules
      (see above) and enough language stability that packages wouldn't
      immediately break across versions. Long-term goal, not near-term.
