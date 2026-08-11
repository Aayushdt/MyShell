/*
 * parser.c — Shell command parser
 *
 * Converts a flat token array into a Pipeline structure. Handles:
 *   - Pipe operator '|' to split into multiple commands
 *   - I/O redirection: '<' (input), '>' (output), '>>' (append)
 *   - Error detection for malformed input
 */

#include "shell.h"
#include "parser.h"

/*
 * init_command() — Zero-initialize a Command struct.
 */
static void init_command(Command *cmd)
{
    memset(cmd, 0, sizeof(Command));
}

/*
 * parse() — Main parser function.
 *
 * Walks through tokens left-to-right, building Command structs.
 * On encountering '|', finalizes the current command and starts a new one.
 */
int parse(char **tokens, int num_tokens, Pipeline *pipeline)
{
    if (num_tokens == 0)
        return -1;

    memset(pipeline, 0, sizeof(Pipeline));
    pipeline->num_commands = 0;

    /* Start first command */
    int cmd_idx = 0;
    init_command(&pipeline->commands[cmd_idx]);

    for (int i = 0; i < num_tokens; i++) {
        /* ── Pipe operator ──────────────────────────────────────────── */
        if (strcmp(tokens[i], "|") == 0) {
            /* Error: pipe at start, end, or empty command between pipes */
            if (pipeline->commands[cmd_idx].argc == 0) {
                fprintf(stderr, SHELL_NAME ": syntax error near '|'\n");
                return -1;
            }
            /* NULL-terminate the args array for execvp */
            pipeline->commands[cmd_idx].args[
                pipeline->commands[cmd_idx].argc] = NULL;

            /* Bounds check: too many commands */
            cmd_idx++;
            if (cmd_idx >= MAX_COMMANDS) {
                fprintf(stderr, SHELL_NAME
                        ": too many commands in pipeline (max %d)\n",
                        MAX_COMMANDS);
                return -1;
            }
            init_command(&pipeline->commands[cmd_idx]);
        }
        /* ── Input redirection ──────────────────────────────────────── */
        else if (strcmp(tokens[i], "<") == 0) {
            if (i + 1 >= num_tokens) {
                fprintf(stderr, SHELL_NAME
                        ": syntax error: expected filename after '<'\n");
                return -1;
            }
            pipeline->commands[cmd_idx].input_file = tokens[++i];
        }
        /* ── Output redirection (append) ────────────────────────────── */
        else if (strcmp(tokens[i], ">>") == 0) {
            if (i + 1 >= num_tokens) {
                fprintf(stderr, SHELL_NAME
                        ": syntax error: expected filename after '>>'\n");
                return -1;
            }
            pipeline->commands[cmd_idx].output_file = tokens[++i];
            pipeline->commands[cmd_idx].append = 1;
        }
        /* ── Output redirection (truncate) ──────────────────────────── */
        else if (strcmp(tokens[i], ">") == 0) {
            if (i + 1 >= num_tokens) {
                fprintf(stderr, SHELL_NAME
                        ": syntax error: expected filename after '>'\n");
                return -1;
            }
            pipeline->commands[cmd_idx].output_file = tokens[++i];
            pipeline->commands[cmd_idx].append = 0;
        }
        /* ── Regular argument ───────────────────────────────────────── */
        else {
            if (pipeline->commands[cmd_idx].argc >= MAX_ARGS - 1) {
                fprintf(stderr, SHELL_NAME
                        ": too many arguments (max %d)\n", MAX_ARGS - 1);
                return -1;
            }
            pipeline->commands[cmd_idx].args[
                pipeline->commands[cmd_idx].argc++] = tokens[i];
        }
    }

    /* Finalize last command */
    if (pipeline->commands[cmd_idx].argc == 0) {
        /* Edge case: trailing pipe "ls |" already caught above,
         * but also handles entirely empty input after redirects */
        if (cmd_idx > 0) {
            fprintf(stderr, SHELL_NAME ": syntax error near '|'\n");
            return -1;
        }
        return -1; /* empty command */
    }
    pipeline->commands[cmd_idx].args[
        pipeline->commands[cmd_idx].argc] = NULL;
    pipeline->num_commands = cmd_idx + 1;

    return 0;
}
