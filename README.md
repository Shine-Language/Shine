# Shine

A small compiled language with its own lexer, parser, and codegen, targeting LLVM IR.

Shine compiles down to native object files and links straight to an executable. No interpreter, no VM, no runtime.

Status: v0.1.0 (unpublished) - compiles functions with parameters, calls, and string/int literals.

---

## Features

**Native compilation** - Shine source goes through a hand-written lexer, recursive-descent parser, and LLVM-based codegen, then straight to an object file and a linked executable.

**Functions with parameters** - `fn int add(a: int, b: int) { ... }`. Parameters are passed as real LLVM arguments and referenced by name in the function body.

**No runtime** - Shine programs link against libc and nothing else. There's no garbage collector, no interpreter loop, no bytecode.

---

## Language (v0.1.0)

- `fn type name(param: type, ...) { ... }`
- `int` / `void` types
- int and string literals
- function calls, including passing arguments
- `r/` (return)
- `write(string)` - a compiler builtin for now, lowers to `puts`

No `let`, no operators, no control flow yet - see `ROADMAP.md`.

```
fn int identity(x: int) {
    r/x;
}

fn int main() {
    write("Hello, World!");
    r/identity(0);
}
```

---

## Build (Windows, MSYS2/MinGW)

```
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake \
          mingw-w64-x86_64-llvm mingw-w64-x86_64-ninja
cmake -B build -G Ninja
cmake --build build
./build/tests/shine_tests.exe
./build/shinec.exe examples/hello.shine -o hello.exe
./hello.exe
```

---

## Layout

```
include/shine/   headers
src/             lexer, parser, codegen, driver (main.cpp)
tests/           unit tests
examples/        sample .shine programs
```

---

## Roadmap

See [ROADMAP.md](ROADMAP.md) for what's done and what's next.
