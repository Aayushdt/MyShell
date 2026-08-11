/*
 * main.c — MyShell entry point and REPL loop
 *
 * Implements the Read-Eval-Print Loop:
 *   1. Print colored prompt with current directory
 *   2. Read a line of input (handle Ctrl+D gracefully)
 *   3. Tokenize → Parse → Execute
 *   4. Repeat
 *
 * Signal handling: SIGINT is ignored in the parent so Ctrl+C
 * doesn't kill the shell. Children reset SIGINT to SIG_DFL.
 */

#include "shell.h"
#include "tokenizer.h"
#include "parser.h"
#include "executor.h"

/*
 * print_prompt() — Display the shell prompt.
 *
 * Format: myshell:~/current/dir$
 * Abbreviates $HOME as ~ for readability.
 */
static void print_prompt(void)
{
    char cwd[MAX_LINE];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        /* Fallback if getcwd fails */
        printf(COLOR_GREEN SHELL_NAME COLOR_RESET ":"
               COLOR_BLUE "?" COLOR_RESET "$ ");
        fflush(stdout);
        return;
    }

    /* Abbreviate $HOME as ~ */
    const char *home = getenv("HOME");
    const char *display_path = cwd;
    char abbrev[MAX_LINE];

    if (home && strncmp(cwd, home, strlen(home)) == 0) {
        snprintf(abbrev, sizeof(abbrev), "~%s", cwd + strlen(home));
        display_path = abbrev;
    }

    printf(COLOR_GREEN SHELL_NAME COLOR_RESET ":"
           COLOR_BLUE "%s" COLOR_RESET "$ ", display_path);
    fflush(stdout);
}

/*
 * main() — Shell entry point.
 */
int main(void)
{
    /*
     * Ignore SIGINT in the parent shell process.
     * Ctrl+C should kill the running child, not the shell itself.
     * Children will call signal(SIGINT, SIG_DFL) after fork()
     * to restore default behavior — this is critical because
     * fork() inherits signal dispositions.
     */
    signal(SIGINT, SIG_IGN);

    char line[MAX_LINE];

    while (1) {
        /* ── Print prompt ───────────────────────────────────────────── */
        print_prompt();

        /* ── Read input ─────────────────────────────────────────────── */
        if (fgets(line, MAX_LINE, stdin) == NULL) {
            /* EOF (Ctrl+D) — exit gracefully */
            printf("\n");
            break;
        }

        /* Skip empty lines */
        if (line[0] == '\n')
            continue;

        /* ── Tokenize ───────────────────────────────────────────────── */
        char *tokens[MAX_TOKENS];
        int num_tokens = tokenize(line, tokens, MAX_TOKENS);
        if (num_tokens <= 0)
            continue;

        /* ── Parse ──────────────────────────────────────────────────── */
        Pipeline pipeline;
        if (parse(tokens, num_tokens, &pipeline) < 0) {
            free_tokens(tokens, num_tokens);
            continue;
        }

        /* ── Execute ────────────────────────────────────────────────── */
        execute(&pipeline);

        /* ── Cleanup ────────────────────────────────────────────────── */
        free_tokens(tokens, num_tokens);
    }

    return 0;
}
