/*
 * executor.c — Shell command executor
 *
 * Handles three execution modes:
 *   1. Built-in commands (parent process)
 *   2. Simple external commands (single fork/exec)
 *   3. Pipelines (multiple fork/exec with pipe() wiring)
 */

#include "shell.h"
#include "builtins.h"
#include "executor.h"

/*
 * setup_redirections() — Apply I/O redirections for a command.
 *
 * Called inside the child process after fork(), before execvp().
 * Opens files and uses dup2() to rewire stdin/stdout.
 *
 * Returns 0 on success, -1 on error.
 */
static int setup_redirections(Command *cmd)
{
    /* Input redirection: cmd < file */
    if (cmd->input_file) {
        int fd = open(cmd->input_file, O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, SHELL_NAME ": %s: %s\n",
                    cmd->input_file, strerror(errno));
            return -1;
        }
        if (dup2(fd, STDIN_FILENO) < 0) {
            perror(SHELL_NAME ": dup2");
            close(fd);
            return -1;
        }
        close(fd);
    }

    /* Output redirection: cmd > file  or  cmd >> file */
    if (cmd->output_file) {
        int flags = O_WRONLY | O_CREAT;
        flags |= cmd->append ? O_APPEND : O_TRUNC;

        int fd = open(cmd->output_file, flags, 0644);
        if (fd < 0) {
            fprintf(stderr, SHELL_NAME ": %s: %s\n",
                    cmd->output_file, strerror(errno));
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror(SHELL_NAME ": dup2");
            close(fd);
            return -1;
        }
        close(fd);
    }

    return 0;
}

/*
 * execute_simple() — Fork and exec a single external command.
 *
 * 1. fork()
 * 2. Child: reset SIGINT, apply redirections, execvp()
 * 3. Parent: waitpid()
 */
static int execute_simple(Command *cmd)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror(SHELL_NAME ": fork");
        return -1;
    }

    if (pid == 0) {
        /* ── Child process ──────────────────────────────────────────── */

        /* Reset SIGINT to default so Ctrl+C kills the child.
         * The parent has SIGINT set to SIG_IGN, and fork() inherits
         * signal dispositions — without this reset, Ctrl+C would be
         * ignored in the child too. */
        signal(SIGINT, SIG_DFL);

        /* Apply I/O redirections */
        if (setup_redirections(cmd) < 0)
            _exit(1);

        /* Replace child process with the target program */
        execvp(cmd->args[0], cmd->args);

        /* If execvp returns, it failed */
        fprintf(stderr, SHELL_NAME ": %s: %s\n",
                cmd->args[0], strerror(errno));
        _exit(127);
    }

    /* ── Parent process ─────────────────────────────────────────────── */
    int status;
    waitpid(pid, &status, 0);

    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

/*
 * execute_pipeline() — Execute a chain of commands connected by pipes.
 *
 * For N commands, creates N-1 pipes. Each command reads from the
 * previous pipe and writes to the next one. The first and last
 * commands connect to the terminal's stdin/stdout respectively
 * (unless redirected).
 *
 * CRITICAL: Every child closes ALL pipe fds it doesn't use.
 * Forgetting this causes read() to block forever because the
 * write-end stays open.
 */
static int execute_pipeline(Pipeline *pipeline)
{
    int n = pipeline->num_commands;
    int pipes[MAX_COMMANDS - 1][2]; /* pipes[i] connects cmd[i] -> cmd[i+1] */
    pid_t pids[MAX_COMMANDS];

    /* Create all pipes upfront */
    for (int i = 0; i < n - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror(SHELL_NAME ": pipe");
            /* Close any pipes we already created */
            for (int j = 0; j < i; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            return -1;
        }
    }

    /* Fork a child for each command */
    for (int i = 0; i < n; i++) {
        pids[i] = fork();

        if (pids[i] < 0) {
            perror(SHELL_NAME ": fork");
            return -1;
        }

        if (pids[i] == 0) {
            /* ── Child process ──────────────────────────────────────── */

            /* Reset SIGINT so Ctrl+C kills pipeline children */
            signal(SIGINT, SIG_DFL);

            /* Wire up pipe ends */
            if (i > 0) {
                /* Not first: read stdin from previous pipe */
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if (i < n - 1) {
                /* Not last: write stdout to next pipe */
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            /* Close ALL pipe fds — child only uses dup2'd copies */
            for (int j = 0; j < n - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            /* Apply per-command I/O redirections */
            if (setup_redirections(&pipeline->commands[i]) < 0)
                _exit(1);

            /* Execute */
            execvp(pipeline->commands[i].args[0],
                   pipeline->commands[i].args);

            fprintf(stderr, SHELL_NAME ": %s: %s\n",
                    pipeline->commands[i].args[0], strerror(errno));
            _exit(127);
        }
    }

    /* ── Parent: close all pipe fds ─────────────────────────────────── */
    for (int i = 0; i < n - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    /* Wait for all children */
    int status;
    for (int i = 0; i < n; i++) {
        waitpid(pids[i], &status, 0);
    }

    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

/*
 * execute() — Main dispatch function.
 *
 * Routes to builtin, simple, or pipeline execution.
 */
int execute(Pipeline *pipeline)
{
    if (pipeline->num_commands == 0)
        return -1;

    /* Check if first (and only) command is a builtin.
     * Builtins only apply to simple commands, not pipelines. */
    if (pipeline->num_commands == 1 &&
        is_builtin(pipeline->commands[0].args[0])) {
        return execute_builtin(&pipeline->commands[0]);
    }

    /* External command(s) */
    if (pipeline->num_commands == 1) {
        return execute_simple(&pipeline->commands[0]);
    }

    return execute_pipeline(pipeline);
}
