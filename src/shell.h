/*
 * shell.h — Shared types, constants, and macros for MyShell
 *
 * Central header included by every translation unit. Defines the two-level
 * command representation: Command (single program) and Pipeline (chain of
 * commands connected by pipes).
 */

/* Enable POSIX extensions (strdup, etc.) in strict C11 mode */
#define _POSIX_C_SOURCE 200809L

#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

/* ── Limits ─────────────────────────────────────────────────────────── */
#define MAX_LINE      1024   /* max input line length                   */
#define MAX_TOKENS    128    /* max tokens per line                     */
#define MAX_ARGS      64     /* max arguments per single command        */
#define MAX_COMMANDS  16     /* max commands in a pipeline              */

/* ── Shell name (used in prompts and error messages) ────────────────── */
#define SHELL_NAME    "myshell"

/* ── ANSI color codes for the prompt ────────────────────────────────── */
#define COLOR_BOLD    "\033[1m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_BLUE    "\033[1;34m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_RESET   "\033[0m"

/* ── Data structures ────────────────────────────────────────────────── */

/*
 * Represents a single command (one segment of a pipeline).
 * Example: in "grep foo < input.txt | sort > out.txt",
 *   Command 0: args=["grep","foo"], input_file="input.txt"
 *   Command 1: args=["sort"],       output_file="out.txt"
 */
typedef struct {
    char *args[MAX_ARGS];    /* NULL-terminated argument array for execvp */
    int   argc;              /* number of arguments                      */
    char *input_file;        /* redirect stdin from file  (<)            */
    char *output_file;       /* redirect stdout to file   (> or >>)      */
    int   append;            /* 1 if >> (append mode), 0 if > (truncate) */
} Command;

/*
 * Represents an entire parsed line (potentially a pipeline).
 * num_commands == 1 means a simple command; > 1 means a pipeline.
 */
typedef struct {
    Command commands[MAX_COMMANDS];
    int     num_commands;
} Pipeline;

#endif /* SHELL_H */
