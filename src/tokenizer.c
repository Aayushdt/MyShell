/*
 * tokenizer.c — Shell input tokenizer
 *
 * Converts a raw input string into an array of token strings.
 * Handles: whitespace splitting, quoted strings, and special
 * characters (|, <, >, >>) as standalone tokens.
 */

#include "shell.h"
#include "tokenizer.h"

/* ── Helper: is this character a special shell operator? ─────────────── */
static int is_special(char c)
{
    return (c == '|' || c == '<' || c == '>');
}

/* ── Helper: duplicate a substring [start, start+len) ───────────────── */
static char *substr_dup(const char *start, int len)
{
    char *s = malloc(len + 1);
    if (!s) {
        perror(SHELL_NAME ": malloc");
        return NULL;
    }
    memcpy(s, start, len);
    s[len] = '\0';
    return s;
}

/*
 * tokenize() — Main tokenizer function.
 *
 * Walks through the input character-by-character, producing tokens.
 * Returns -1 on error (unterminated quote, overflow).
 */
int tokenize(const char *line, char **tokens, int max_tokens)
{
    int i = 0;
    int count = 0;
    int len = strlen(line);

    while (i < len) {
        /* Skip whitespace */
        while (i < len && (line[i] == ' ' || line[i] == '\t' ||
                           line[i] == '\n' || line[i] == '\r'))
            i++;

        if (i >= len)
            break;

        /* Bounds check: too many tokens */
        if (count >= max_tokens) {
            fprintf(stderr, SHELL_NAME ": too many tokens (max %d)\n",
                    max_tokens);
            return -1;
        }

        /* ── Quoted string ──────────────────────────────────────────── */
        if (line[i] == '"') {
            i++; /* skip opening quote */
            int start = i;
            while (i < len && line[i] != '"')
                i++;
            if (i >= len) {
                fprintf(stderr, SHELL_NAME ": unterminated quote\n");
                return -1;
            }
            tokens[count] = substr_dup(&line[start], i - start);
            if (!tokens[count]) return -1;
            count++;
            i++; /* skip closing quote */
        }
        /* ── >> (append redirect) ───────────────────────────────────── */
        else if (line[i] == '>' && (i + 1) < len && line[i + 1] == '>') {
            tokens[count] = strdup(">>");
            if (!tokens[count]) return -1;
            count++;
            i += 2;
        }
        /* ── Single special character: |, <, > ─────────────────────── */
        else if (is_special(line[i])) {
            tokens[count] = substr_dup(&line[i], 1);
            if (!tokens[count]) return -1;
            count++;
            i++;
        }
        /* ── Regular word ───────────────────────────────────────────── */
        else {
            int start = i;
            while (i < len && line[i] != ' '  && line[i] != '\t' &&
                   line[i] != '\n' && line[i] != '\r' &&
                   line[i] != '"'  && !is_special(line[i]))
                i++;
            tokens[count] = substr_dup(&line[start], i - start);
            if (!tokens[count]) return -1;
            count++;
        }
    }

    return count;
}

/*
 * free_tokens() — Release all token memory.
 */
void free_tokens(char **tokens, int num_tokens)
{
    for (int i = 0; i < num_tokens; i++) {
        free(tokens[i]);
        tokens[i] = NULL;
    }
}
