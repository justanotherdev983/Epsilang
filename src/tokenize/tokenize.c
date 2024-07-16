#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>


int totalTokens;

typedef enum TokenType
{
    type_exit = 0, // no need for this, automatically done
    type_int_lit = 1,
    type_semi = 2,
    type_space = 3,
    type_EOF = 4,
} TokenType;

typedef struct Token
{
    TokenType type;
    char *value;
} Token;

void throw_error(char message[])
{
  fprintf(stderr, "%s", message);
}

void consume(char contents[], int *token_index, int length)
{

  *token_index += length;

  if (*token_index >= strlen(contents))
  {
    *token_index = strlen(contents);
  }
}

char peek(char contents[], int token_index)
{

  if (token_index >= strlen(contents))
  {
    return '\0';
  }
  else
  {
    return contents[token_index]; // Value not used; interesting
  }
}

bool has_value(char c)
{
  return (c != '\0');
}

bool match_string(char contents[], int token_index, const char *str)
{
  int len = strlen(str);
  for (int i = 0; i < len; i++)
  {
    if (peek(contents, token_index + i) != str[i])
    {
      return false;
    }
  }
  return true;
}

Token *tokenize(char contents[])
{
  Token *tokens = malloc(100 * sizeof(Token)); // needs to change dynamically with sizeof(contents)
  int currentTokenIndex = 0; // Really weird, never inrement index but works?
  Token currentToken;

  totalTokens = 0;

  while (has_value(peek(contents, currentTokenIndex)))
  {
    if (isdigit(peek(contents, currentTokenIndex)))
    {
      currentToken.type = type_int_lit;
      currentToken.value = malloc(20 * sizeof(char));
      int i = 0;
      while (isdigit(peek(contents, currentTokenIndex)))
      {
        currentToken.value[i++] = peek(contents, currentTokenIndex);
        consume(contents, &currentTokenIndex, 1);
      }
      currentToken.value[i] = '\0';
    }
    else if (isalpha(peek(contents, currentTokenIndex)))
    {
      if (match_string(contents, currentTokenIndex, "exit"))
      {
        currentToken.type = type_exit;
        consume(contents, &currentTokenIndex, 4);
      }
      else
      {
        throw_error("Invalid keyword");
        exit(EXIT_FAILURE);
      }
    }
    else if (peek(contents, currentTokenIndex) == ';')
    {
      currentToken.type = type_semi;
      consume(contents, &currentTokenIndex, 1);
    }
    else if (isspace(peek(contents, currentTokenIndex)))
    {
      currentToken.type = type_space;
      while (isspace(peek(contents, currentTokenIndex)))
      {
        consume(contents, &currentTokenIndex, 1);
      }
    }
    else
    {
      throw_error("Invalid token");
      exit(EXIT_FAILURE);
    }

    if (totalTokens < 100) // should be dynamic again
    {
      tokens[totalTokens++] = currentToken;
    }
    else
    {
      currentToken.type = type_EOF;
      throw_error("Maximum number of tokens reached");
      exit(EXIT_FAILURE);
    }
  }

  for (int i = 0; i < totalTokens; ++i)
  {
    printf("Token %d: Type=%d\n", i, tokens[i].type);
  }


  return tokens;
}



int Lexer(char **argv, char contents[])
{

  FILE *fileptr;

  fileptr = fopen(argv[1], "r");

  if (fileptr == NULL)
  {
    perror("Error opening file");
    return 1;
  }

  fgets(contents, 100, fileptr);

  fclose(fileptr);

  Token *tokens = tokenize(contents); // Call it here and in main func???
  free(tokens);

  return 0;
}
