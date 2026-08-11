# MyShell 🐚

A mini Unix shell written in C that demonstrates core OS concepts: process creation, inter-process communication, and file descriptor manipulation.

## Features

- **REPL Loop** — Interactive read-eval-print loop with a colored prompt showing the current directory
- **External Commands** — Run any program via `fork()` + `execvp()` (ls, grep, cat, etc.)
- **Built-in Commands** — `cd`, `pwd`, `exit`, `help` (executed in the parent process)
- **Pipe Support** — Chain commands with `|` (e.g., `ls -la | grep .c | wc -l`)
- **I/O Redirection** — `<` (input), `>` (output/truncate), `>>` (output/append)
- **Signal Handling** — Ctrl+C kills the running child, not the shell; Ctrl+D exits gracefully

## Syscalls Used

| Syscall | Purpose |
|---|---|
| `fork()` | Create child processes |
| `execvp()` | Load and run external programs |
| `waitpid()` | Synchronize parent with child |
| `pipe()` | Inter-process communication |
| `dup2()` | Redirect file descriptors |
| `open()` / `close()` | File I/O for redirections |
| `chdir()` / `getcwd()` | Directory navigation |
| `signal()` | Handle Ctrl+C (SIGINT) |

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

## Example Session

```
myshell:~/Projects$ echo Hello, World!
Hello, World!
myshell:~/Projects$ ls -la | grep .c | wc -l
5
myshell:~/Projects$ echo hello > output.txt
myshell:~/Projects$ cat < output.txt
hello
myshell:~/Projects$ cd /tmp
myshell:/tmp$ pwd
/tmp
myshell:/tmp$ exit
```

## Architecture

```
Input → tokenize() → parse() → execute()
                                   ├── execute_builtin()   (cd, pwd, exit)
                                   ├── execute_simple()    (single fork/exec)
                                   └── execute_pipeline()  (pipe chain)
```

| File | Responsibility |
|---|---|
| `src/main.c` | Entry point, REPL loop, signal setup |
| `src/tokenizer.c` | Raw input → token array |
| `src/parser.c` | Token array → Pipeline struct |
| `src/executor.c` | fork/exec/pipe/dup2 logic |
| `src/builtins.c` | cd, pwd, exit, help |
| `src/shell.h` | Shared types and constants |

## License

MIT
