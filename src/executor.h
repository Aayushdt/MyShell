/*
 * executor.h — Interface for the shell command executor
 */

#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "shell.h"

/*
 * execute() — Execute a parsed pipeline.
 *
 * Dispatches to the appropriate execution path:
 *   - Built-in commands (run in parent process)
 *   - Simple commands (single fork/exec)
 *   - Pipelines (multiple fork/exec with pipe() wiring)
 *
 * @param pipeline  The parsed Pipeline struct.
 * @return 0 on success, -1 on error.
 */
int execute(Pipeline *pipeline);

#endif /* EXECUTOR_H */
