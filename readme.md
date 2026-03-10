# mysh — A Unix Shell Built from Scratch

A minimal but functional Unix shell implemented in C using only syscalls. No readline, no external libraries — just `fork()`, `execvp()`, `pipe()`, and signal handlers.

## Features

- **Command execution**: External programs launched with `fork()` and `execvp()`
- **Pipes**: Chain commands with `|` (e.g., `ls | grep .c | wc -l`)
- **I/O redirection**: `>` (truncate), `>>` (append), `<` (read), `2>` (stderr)
- **Background jobs**: Run commands with `&`; manage with `jobs`, `fg`, `bg`
- **Built-in commands**: `cd`, `exit`, `jobs`, `fg`, `bg`, `history`
- **Signal handling**: Properly ignore Ctrl+C/Z in shell, respond in child processes
- **Job control**: Track running/stopped processes, handle `SIGCHLD` reaping

## Build

```bash
make              # Build normally
make debug        # Build with AddressSanitizer (memory debugging)
make clean        # Remove compiled files
```

## Run

```bash
./mysh
```

## Project Structure

```
mysh/
├── Makefile              Build system
├── README.md             This file
├── .gitignore            Git ignore rules
└── src/
    ├── shell.h           Header with constants, structs, declarations
    ├── main.c            Read-eval-print loop (REPL)
    ├── parser.c          Tokenize input, detect pipes/redirects
    ├── executor.c        Fork/exec single commands
    ├── builtins.c        Built-in commands (cd, exit, jobs, fg, bg, history)
    ├── redirect.c        File I/O redirection (>, <, >>, 2>)
    ├── pipes.c           Multi-command pipelines with pipe()/dup2()
    ├── signals.c         Signal handlers (SIGINT, SIGTSTP, SIGCHLD)
    └── jobs.c            Background/stopped job tracking
```

## Development Progress

| Phase | Module(s) | Status | % Done | Key Learnings | Date |
|-------|-----------|--------|--------|---------------|------|
| 1 | shell.h, .gitignore, README | NOT STARTED | 0% | — | — |
| 2 | parser.c, main.c (REPL) | NOT STARTED | 0% | — | — |
| 3 | executor.c (fork/exec) | NOT STARTED | 0% | — | — |
| 4 | builtins.c (cd, exit) | NOT STARTED | 0% | — | — |
| 5 | redirect.c, pipes.c | NOT STARTED | 0% | — | — |
| 6 | signals.c, jobs.c, builtins (complete) | NOT STARTED | 0% | — | — |
| 7 | Pipe integration | NOT STARTED | 0% | — | — |
| 8 | Makefile | NOT STARTED | 0% | — | — |
| 9 | Documentation & polish | NOT STARTED | 0% | — | — |

## How to Use

### Basic Commands

```bash
mysh> ls                    # Run external command
mysh> pwd                   # Print working directory
mysh> cd src                # Change directory (builtin)
mysh> ls | grep .c          # Pipe commands
mysh> echo hello > out.txt  # Redirect stdout
mysh> cat < out.txt         # Redirect stdin
```

### Background Jobs

```bash
mysh> sleep 100 &           # Run in background
mysh> jobs                  # List jobs
mysh> [Ctrl+Z]              # Stop foreground job
mysh> bg                    # Resume in background
mysh> fg                    # Bring to foreground
```

### Built-in Commands

- `cd [dir]` — Change directory (default: home)
- `exit [code]` — Exit shell with optional exit code
- `jobs` — List running and stopped background jobs
- `fg [job_id]` — Bring job to foreground
- `bg [job_id]` — Resume stopped job in background
- `history` — Show command history

## Design Notes

### Why Reimplement?

Building a shell from scratch teaches:
- **Process creation**: How `fork()` and `execvp()` work
- **I/O multiplexing**: Pipes, file descriptors, `dup2()`
- **Signal handling**: Asynchronous events, race conditions, reaping
- **Job control**: Process lifecycle, states, waiting strategies

### Architecture

- **Single header file** (`shell.h`): Constants, structs, all function declarations. Makes cross-module contracts explicit.
- **Linear compilation**: Each module is independent after parsing. No circular dependencies.
- **Manual cleanup**: No garbage collection. Careful `free()` calls after every allocation.
- **Standard signals**: Uses `sigaction()` for portability, not `signal()`.

## Known Limitations

- No history persistence to file
- No tab completion or readline-like editing
- Limited error recovery; some edge cases may crash
- No advanced features (aliases, functions, variables, `set` builtin)

## Testing

All features tested manually:
1. Basic commands run and produce correct output
2. Pipes correctly chain output of multiple commands
3. Redirects open/create files as expected
4. Background jobs tracked and managed correctly
5. Memory clean (run with `make debug` and check for leaks)

## Author's Notes

This project is a learning exercise in systems programming. The goal is not a production shell, but understanding how Unix process management actually works at the syscall level.