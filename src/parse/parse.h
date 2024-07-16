#ifndef PARSE_H
#define PARSE_H

#include <stdbool.h>
#include "../tokenize/tokenize.h"

void parse_to_asm(Token *curr_token);
bool match(TokenType expected_token, Token *curr_token);
Token *get_next_token(Token *tokens);
TokenType peek_token(Token *tokens);
void consume_token();
void parse_exit_stmt();
void parse_tokens(Token *tokens);

#endif
