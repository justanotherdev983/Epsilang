#include <iostream>
#include <fstream>
#include <string>

#include "core/parse.hpp"
#include "core/tokenise.hpp"
#include "core/codegen.hpp"
#include "utils/error.hpp"



std::string program_contents;



int main(int argc, char **argv)
{
  if (argc != 2)
  {
    std::cout << "Incorrect usage, please specify the file\n";
    std::cout << "Correct usage: ./epsilang <Filename.eps>\n";
    return 1;
  }

  std::ofstream output_asm("../output/output.asm");
  if (!output_asm) {
    std::cerr << "Error: Could not open output file '../output/output.asm'\n";
    return 1;
  }

  std::ifstream input_file(argv[1]);
  if (!input_file) {
    std::cerr << "Error: Could not open file " << argv[1] << "\n";
    return 1;
  }

  std::getline(input_file, program_contents);
  std::cout << "File contents: " << program_contents << "\n";




  std::vector<token_t> tokens = tokenise(program_contents);
  std::vector<ast_node_t> ast = parse_statement(tokens);

  gen_code_for_ast(ast, output_asm);

  std::cout << "boop" << std::endl;
  system("nasm -f elf64 -o ../output/output.o ../output/output.asm");
  system("ld -o ../output/output ../output/output.o");
  system("../output/output");


  return 0;
}
