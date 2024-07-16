#include <stdio.h>
#include <stdlib.h>

#include "parse/parse.h"
#include "tokenize/tokenize.h"

char contents[100]; // TODO: make pointer no 100 limit, bad name
int main(int argc, char **argv)
{
  fopen("../output/output.asm", "w");


  if (argc != 2)
  {
    printf("Incorrect usage, please specify the file\n");
    printf("Correct usage: ./z <Filename>\n");
    return 1;
  }

  FILE *file = fopen(argv[1], "r");
  if (!file)
  {
    printf("Error: Could not open file %s\n", argv[1]);
    return 1;
  }

  fgets(contents, sizeof(contents), file);
  fclose(file);

  Token *tokens = tokenize(contents);

  parse_tokens(tokens);

  system("nasm -f elf64 -o ../output/output.o ../output/output.asm");
  system("ld -o ../output/output ../output/output.o");

  free(tokens);

  return 0;
}
