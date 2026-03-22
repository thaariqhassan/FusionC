# FusionC — Intelligent Multi-Language Compiler

FusionC is a teaching-friendly compiler pipeline that can parse, semantically check, lower to three-address code, run a constant-folding optimizer, and interpret the program. Current support covers a subset of C (single function, declarations, assignments, arithmetic, returns) and the provided CustomLang sample grammar.

## Capabilities
- Language detection (extension + optional hint)
- Lexical analysis ([frontend/lexer/token.cpp](frontend/lexer/token.cpp))
- Parsing to AST with functions/blocks/returns ([frontend/parser/ast.cpp](frontend/parser/ast.cpp))
- Semantic checks with scoped symbols and basic return-type validation ([frontend/semantic/symbol_table.cpp](frontend/semantic/symbol_table.cpp))
- TAC generation ([middleend/ir/three_address_code.cpp](middleend/ir/three_address_code.cpp))
- Constant folding optimizer ([middleend/optimizer/optimizer.cpp](middleend/optimizer/optimizer.cpp))
- TAC interpreter backend ([backend/codegen/machine_codegen.cpp](backend/codegen/machine_codegen.cpp))

## Architecture Pipeline
1) **Language detection** — [core/language_loader.cpp](core/language_loader.cpp)
2) **Lexing** — produces tokens
3) **Parsing** — builds AST with functions/blocks/expressions
4) **Semantic analysis** — symbol table, duplicate/undefined checks, return type consistency
5) **IR build** — three-address code (const, copy, add, sub, mul, div, ret)
6) **Optimization** — constant folding
7) **Execution** — TAC interpreter; reports exit code

## Project Layout (key files)
- Entry: [main.cpp](main.cpp)
- CLI: [fusionc_cli/cli_parser.cpp](fusionc_cli/cli_parser.cpp)
- Controller: [core/compiler_controller.cpp](core/compiler_controller.cpp)
- Frontend: lexer/parser/semantic under `frontend/`
- Middle-end: TAC + optimizer under `middleend/`
- Backend: interpreter under `backend/codegen/`
- Samples: [tests/test_c/sample.c](tests/test_c/sample.c), [tests/test_custom/sample.fsc](tests/test_custom/sample.fsc)

## Build (Windows)
Requires CMake and MSVC build tools.

```powershell
cmake -S . -B build
cmake --build build
```

Result: `build\fusionc.exe`

## CLI Usage

```
fusionc <source-file> <language?> [--dump-tokens] [--dump-ast]
```

- `<language>` is optional; pass `c` or `custom` to override detection.
- `--dump-tokens` prints lexer tokens.
- `--dump-ast` prints AST.

Show help:

```powershell
.\build\fusionc.exe --help
```

## Run Examples

Run C sample with explicit language and diagnostics:

```powershell
.\build\fusionc.exe tests\test_c\sample.c c --dump-tokens --dump-ast
```

Run CustomLang sample:

```powershell
.\build\fusionc.exe tests\test_custom\sample.fsc custom --dump-tokens --dump-ast
```

Observe the printed exit code from the interpreter at the end.

## Development & Testing
- Reconfigure and rebuild after changes: `cmake -S . -B build` then `cmake --build build`.
- Manual smoke test: run the C sample above and verify the reported exit code matches expectations (should be 30 for the provided sample arithmetic).
- Adjust or extend grammar/semantics in `frontend/` and update TAC builder/optimizer/backend accordingly.

## Limitations / Next Steps
- Only arithmetic expressions, declarations, assignments, and returns; no control flow yet.
- Backend is an interpreter, not native codegen; can be replaced with real code generation later.
- Type system is minimal (int-focused) and does not yet validate function params.

## Troubleshooting
- `cmake` not found: reinstall CMake and add to PATH.
- Build fails for missing MSVC: install Visual Studio Build Tools (C++ workload).
- Unexpected language detection: pass the `<language>` hint (`c` or `custom`).

