/*
 * tokenizer.h — Interface for the shell input tokenizer
 */

#ifndef TOKENIZER_H
#define TOKENIZER_H

/*
 * tokenize() — Split a raw input line into an array of token strings.
 *
 * Splits on whitespace. Preserves special characters (|, <, >, >>)
 * as standalone tokens. Handles double-quoted strings as single tokens.
 *
 * @param line       The raw input string (will be read but not modified).
 * @param tokens     Output array of dynamically allocated token strings.
 *                   Caller is responsible for freeing each token.
 * @param max_tokens Maximum number of tokens to store.
 *
 * @return Number of tokens on success, -1 on error (unterminated quote,
 *         too many tokens).
 */
int tokenize(const char *line, char **tokens, int max_tokens);

/*
 * free_tokens() — Free all dynamically allocated token strings.
 *
 * @param tokens     Array of token strings to free.
 * @param num_tokens Number of tokens in the array.
 */
void free_tokens(char **tokens, int num_tokens);

#endif /* TOKENIZER_H */
