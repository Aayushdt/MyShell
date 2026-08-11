/*
 * parser.h — Interface for the shell command parser
 */

#ifndef PARSER_H
#define PARSER_H

#include "shell.h"

/*
 * parse() — Convert a token array into a Pipeline structure.
 *
 * Splits tokens on '|' into separate Command structs. Extracts
 * I/O redirection operators (<, >, >>) and their target filenames.
 *
 * @param tokens     Array of token strings from the tokenizer.
 * @param num_tokens Number of tokens in the array.
 * @param pipeline   Output Pipeline structure to populate.
 *
 * @return 0 on success, -1 on parse error.
 */
int parse(char **tokens, int num_tokens, Pipeline *pipeline);

#endif /* PARSER_H */
