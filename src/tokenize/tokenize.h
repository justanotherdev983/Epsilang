#ifndef TOKENIZE_H
#define TOKENIZE_H

#include <stdbool.h>

typedef enum
{
    type_exit,
    type_int_lit,
    type_space,
    type_semi,
    type_EOF,
} TokenType;

typedef struct
{
    TokenType type;
    char *value;
} Token;

extern char contents[100];
extern int token_index;
extern int totalTokens;

void throw_error(char message[]);
void consume(char contents[], int *token_index, int length);
char peek(char contents[], int token_index);
bool has_value(char c);
bool match_string(char contents[], int token_index, const char *str);
Token *tokenize(char contents[]);
int Lexer(char **argv, char contents[]);

#endif
