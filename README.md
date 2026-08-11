# MyShell 

A mini Unix shell written in C that demonstrates core OS concepts: process creation, inter-process communication, and file descriptor manipulation.

Built from scratch with no external shell dependencies — just the C standard library and POSIX syscalls.

![C](https://img.shields.io/badge/language-C11-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%2F%20POSIX-lightgrey.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

---

## Table of Contents

- [Features](#features)
- [Syscalls Used](#syscalls-used)
- [Build & Run](#build--run)
- [Example Session](#example-session)
- [Architecture](#architecture)
- [Project Structure](#project-structure)
- [Design Notes](#design-notes)
- [Known Limitations](#known-limitations)
- [Testing](#testing)
- [License](#license)

---

## Features

- **REPL Loop** — Interactive read-eval-print loop with a colored prompt showing the current directory
- **External Commands** — Run any program via `fork()` + `execvp()` (`ls`, `grep`, `cat`, etc.)
- **Built-in Commands** — `cd`, `pwd`, `exit`, `help` (executed in the parent process, since they must modify shell state directly)
- **Pipe Support** — Chain any number of commands with `|` (e.g., `ls -la | grep .c | wc -l`)
- **I/O Redirection** — `<` (input), `>` (output/truncate), `>>` (output/append)
- **Quoted Strings** — `echo "hello world"` is treated as a single argument
- **Signal Handling** — Ctrl+C kills the running child, not the shell; Ctrl+D exits gracefully
- **Zero Zombie Processes** — Every forked child is properly reaped with `waitpid()`, including all stages of a pipeline

## Syscalls Used

| Syscall | Purpose |
|---|---|
| `fork()` | Create child processes |
| `execvp()` | Load and run external programs, searching `$PATH` |
| `waitpid()` | Synchronize parent with child, reap zombies |
| `pipe()` | Inter-process communication between pipeline stages |
| `dup2()` | Redirect file descriptors (stdin/stdout to pipes or files) |
| `open()` / `close()` | File I/O for `<`, `>`, `>>` redirections |
| `chdir()` / `getcwd()` | Directory navigation for `cd` / `pwd` |
| `signal()` | Handle `SIGINT` (Ctrl+C) in both the shell and its children |

## Build & Run

```bash
# Build
make

# Run
./myshell

# Run tests
make test

# Clean build artifacts
make clean
```

**Requirements:** GCC (or Clang), a POSIX-compliant environment (Linux, WSL2, or macOS).

## Example Session

```
myshell:~/Projects$ echo Hello, World!
Hello, World!
myshell:~/Projects$ ls -la | grep .c | wc -l
5
myshell:~/Projects$ echo hello > output.txt
myshell:~/Projects$ cat < output.txt
hello
myshell:~/Projects$ echo "hello world" | tr a-z A-Z
HELLO WORLD
myshell:~/Projects$ cd /tmp
myshell:/tmp$ pwd
/tmp
myshell:/tmp$ exit
```

## Architecture

```
Input → tokenize() → parse() → execute()
                                   ├── execute_builtin()   (cd, pwd, exit, help)
                                   ├── execute_simple()    (single fork/exec)
                                   └── execute_pipeline()  (N-stage pipe chain)
```

Each stage is intentionally single-responsibility:

1. **Tokenizer** splits raw input into words, quoted strings, and operators (`|`, `<`, `>`, `>>`).
2. **Parser** turns the token stream into a `Pipeline` struct — one or more `Command`s, each with its own argument list and optional redirections.
3. **Executor** decides whether to run a built-in in-process, fork a single external command, or wire up an N-stage pipeline with `pipe()` + `dup2()`.

## Project Structure

| File | Responsibility |
|---|---|
| `src/main.c` | Entry point, REPL loop, top-level signal setup |
| `src/tokenizer.c` / `.h` | Raw input → token array (handles quoting and operators) |
| `src/parser.c` / `.h` | Token array → `Pipeline` struct (handles `\|`, `<`, `>`, `>>`) |
| `src/executor.c` / `.h` | `fork()` / `execvp()` / `pipe()` / `dup2()` logic |
| `src/builtins.c` / `.h` | `cd`, `pwd`, `exit`, `help` |
| `src/shell.h` | Shared types (`Command`, `Pipeline`) and constants |
| `tests/test_shell.sh` | Automated smoke tests |

## Design Notes

**Signal handling in children.** The shell ignores `SIGINT` (`SIG_IGN`) at the top level so Ctrl+C doesn't kill the shell itself. Because signal dispositions are inherited across `fork()`, every child process explicitly resets `SIGINT` back to `SIG_DFL` before calling `execvp()`. Without this reset, forked commands would silently inherit the ignore-SIGINT behavior and become un-killable with Ctrl+C — this is a common and easy-to-miss bug in hand-rolled shells.

**Pipe file descriptor hygiene.** For an N-command pipeline, N−1 pipes are created up front. Every child closes every pipe file descriptor it doesn't use, and the parent closes each pipe as soon as both of its connected children have been forked. Leaving an unused write-end open anywhere causes downstream `read()` calls to block forever waiting for EOF that never comes — this is the single most common bug in pipeline implementations, and this shell is written to avoid it deliberately, not by accident.

**Builtins run in-process.** `cd`, `pwd`, `exit`, and `help` execute directly in the shell's own process rather than being forked. This is necessary — a forked `cd` would only change the child's working directory, not the shell's, and a forked `exit` would only terminate the child, not the shell itself.

**Child failure handling.** If `execvp()` fails (e.g., unknown command), the child prints an error with `perror()` and terminates with `_exit()`, not `exit()`. Using `exit()` here would flush the parent's inherited stdio buffers a second time, producing duplicate output.

## Known Limitations

This project intentionally scopes to the core OS concepts rather than reimplementing a full-featured shell. Not currently supported:

- Background execution (`cmd &`)
- Command history / arrow-key recall
- Environment variable expansion (`$HOME`, `$PATH`, etc.)
- Shell scripting constructs (`if`, loops, `&&`, `||`)
- Job control (`fg`, `bg`, `jobs`)

## Testing

`tests/test_shell.sh` pipes a series of commands into the built binary and checks the output against expected results, covering builtins, pipes, and redirection. Run it with:

```bash
make test
```

For deeper verification:

```bash
# Check for zombie processes after a session
ps aux | grep myshell

# Check for memory leaks
valgrind --leak-check=full ./myshell
```

## License

MIT
