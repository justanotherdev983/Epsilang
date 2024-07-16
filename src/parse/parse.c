#include <stdio.h>
#include <stdlib.h>
#include "../tokenize/tokenize.h"
#include "../parse/parse.h"

int TokenIndex = 0;
void parse_to_asm(Token *curr_token)
{
  FILE *fileptr = fopen("../output/output.asm", "w");
  if (fileptr == NULL)
  {
    throw_error("Unable to open output asm file");

  }
  else
  {
    printf("opened asm file");
  }
  fprintf(fileptr, "global _start\n");
  fprintf(fileptr, "_start:\n");

  if (curr_token->type == type_int_lit)
  {
    fprintf(fileptr, "    mov rdi, %d\n", atoi(curr_token->value));
  }
  else
  {
    throw_error("Invalid syntax: Expected int_lit\n");
  }

  fprintf(fileptr, "    mov rax, 60\n");
  fprintf(fileptr, "    syscall\n");
  fflush(fileptr);
  fclose(fileptr);
}


Token *get_next_token(Token *tokens)
{
  if (TokenIndex < totalTokens)
  {
    Token *curr_token = &tokens[TokenIndex];
    return curr_token;
  }
  else
  {
    return NULL;
  }
}


void consume_token()
{
  TokenIndex += 1; // Global variable, not like this

  if (TokenIndex >= totalTokens)
  {
    TokenIndex = totalTokens; // This does nothing
  }
}

void parse_exit_stmt(Token **tokens) {
  if (get_next_token(*tokens) == type_exit) {
    printf("Matched _exit\n");
    consume_token();
    Token *next_token = get_next_token(*tokens);
    if (next_token != NULL) {
      printf("Next Token after exit: Type=%d, Value=%s\n", next_token->type, next_token->value);
    } else {
      printf("No more tokens after exit\n");
    }
    if (next_token->type == type_space) {
      printf("Matched space\n");
      consume_token();
      next_token = get_next_token(*tokens);

      if (next_token->type == type_int_lit) {
        printf("Matched int_lit\n");
        consume_token();
        next_token = get_next_token(*tokens);

        if (next_token->type == type_semi) {
          printf("Matched semi\n");
          parse_to_asm(next_token);
          consume_token();
        } else {
          throw_error("Expected semicolon after integer literal\n");
        }
      } else {
        throw_error("Expected integer literal after _exit\n");
      }
    } else {
      throw_error("Expected space after _exit\n");
    }
  } else {
    throw_error("Expected _exit statement\n");
  }
}

void parse_tokens(Token *tokens)
{
  for (int i = 0; i < totalTokens; ++i)
  {
    Token *curr_token = &tokens[i];

    if (curr_token->type == type_exit)
    {
      parse_exit_stmt(&curr_token); // Shouldn't this be token stream???
    }
  }
}