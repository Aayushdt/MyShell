/*
 * builtins.h — Interface for shell built-in commands
 */

#ifndef BUILTINS_H
#define BUILTINS_H

#include "shell.h"

/*
 * is_builtin() — Check if a command name is a built-in.
 *
 * @param cmd_name  The command name (first argument).
 * @return 1 if it's a built-in, 0 otherwise.
 */
int is_builtin(const char *cmd_name);

/*
 * execute_builtin() — Execute a built-in command.
 *
 * Must be called in the parent process (not forked) because builtins
 * like 'cd' modify the shell's own state.
 *
 * @param cmd  The parsed Command struct.
 * @return 0 on success, -1 on error, 1 if the shell should exit.
 */
int execute_builtin(Command *cmd);

#endif /* BUILTINS_H */
