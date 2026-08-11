/*
 * builtins.c — Shell built-in command implementations
 *
 * Built-in commands run in the parent process because they modify
 * the shell's own state (e.g., cd changes the working directory).
 *
 * Supported builtins: cd, pwd, exit, help
 */

#include "shell.h"
#include "builtins.h"

/* ── Table of built-in command names ────────────────────────────────── */
static const char *builtin_names[] = {
    "cd",
    "pwd",
    "exit",
    "help",
    NULL
};

/*
 * is_builtin() — Check if cmd_name matches a known built-in.
 */
int is_builtin(const char *cmd_name)
{
    for (int i = 0; builtin_names[i] != NULL; i++) {
        if (strcmp(cmd_name, builtin_names[i]) == 0)
            return 1;
    }
    return 0;
}

/* ── cd [dir] ───────────────────────────────────────────────────────── */
static int builtin_cd(Command *cmd)
{
    const char *dir;

    if (cmd->argc < 2) {
        /* No argument: go to $HOME */
        dir = getenv("HOME");
        if (!dir) {
            fprintf(stderr, SHELL_NAME ": cd: HOME not set\n");
            return -1;
        }
    } else {
        dir = cmd->args[1];
    }

    if (chdir(dir) != 0) {
        fprintf(stderr, SHELL_NAME ": cd: %s: %s\n", dir, strerror(errno));
        return -1;
    }

    return 0;
}

/* ── pwd ────────────────────────────────────────────────────────────── */
static int builtin_pwd(Command *cmd)
{
    (void)cmd; /* unused */

    char cwd[MAX_LINE];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror(SHELL_NAME ": pwd");
        return -1;
    }
    printf("%s\n", cwd);
    return 0;
}

/* ── exit [code] ────────────────────────────────────────────────────── */
static int builtin_exit(Command *cmd)
{
    int code = 0;
    if (cmd->argc >= 2)
        code = atoi(cmd->args[1]);

    /* Return 1 to signal the REPL to exit */
    /* We store the exit code and let main handle the actual exit */
    printf("exit\n");
    exit(code);
    return 1; /* unreachable, but satisfies the compiler */
}

/* ── help ───────────────────────────────────────────────────────────── */
static int builtin_help(Command *cmd)
{
    (void)cmd; /* unused */

    printf("\n");
    printf(COLOR_BOLD "  MyShell" COLOR_RESET
           " — A mini Unix shell written in C\n\n");
    printf("  Built-in commands:\n");
    printf("    cd [dir]     Change working directory (default: $HOME)\n");
    printf("    pwd          Print current working directory\n");
    printf("    exit [code]  Exit the shell with optional exit code\n");
    printf("    help         Show this help message\n");
    printf("\n");
    printf("  Features:\n");
    printf("    cmd1 | cmd2  Pipe output of cmd1 into cmd2\n");
    printf("    cmd < file   Redirect stdin from file\n");
    printf("    cmd > file   Redirect stdout to file (truncate)\n");
    printf("    cmd >> file  Redirect stdout to file (append)\n");
    printf("\n");

    return 0;
}

/*
 * execute_builtin() — Dispatch to the correct built-in handler.
 */
int execute_builtin(Command *cmd)
{
    if (strcmp(cmd->args[0], "cd") == 0)
        return builtin_cd(cmd);
    if (strcmp(cmd->args[0], "pwd") == 0)
        return builtin_pwd(cmd);
    if (strcmp(cmd->args[0], "exit") == 0)
        return builtin_exit(cmd);
    if (strcmp(cmd->args[0], "help") == 0)
        return builtin_help(cmd);

    /* Should not reach here if is_builtin() was checked first */
    fprintf(stderr, SHELL_NAME ": unknown builtin: %s\n", cmd->args[0]);
    return -1;
}
