#include <iostream>
#include <fstream>
#include <iterator>
#include <string>

#include "core/parse.hpp"
#include "core/tokenise.hpp"
#include "core/codegen.hpp"
#include "utils/error.hpp"

/*
fn add(a, b) {
    return a + b;
}

fn max(a, b) {
    if (a >= b) {
        return a;
    } else {
        return b;
    }
}

let result = add(5, 10);
let bigger = max(result, 20);

let counter = 0;
let sum = 0;
while (counter < 5) {
    sum = add(sum, counter);
    counter = counter + 1;
}

let final_result = max(bigger, sum);
exit(final_result);*/

std::string program_contents;



int main(int argc, char **argv)
{
  info_msg("Outputted binary is found in output/output");
  if (argc != 2)
  {
    error_msg("Incorrect usage, please specify the file");
    info_msg("Correct usage is: ./epsilang <Filename.eps>");

    return 1;
  }

  std::ofstream output_asm("../output/output.asm");
  if (!output_asm) {
    error_msg("Could not open output file '../output/output.asm'");
    return 1;
  }

  std::ifstream input_file(argv[1]);
  if (!input_file) {
    error_msg("Could not open file: {}", argv[1]);
    return 1;
  }

  std::string line_buf;
  while (std::getline(input_file, line_buf)) {
    program_contents += line_buf + '\n';
  }

  info_msg("File contents: {}", program_contents);




  std::vector<token_t> tokens = tokenise(program_contents);
  std::vector<ast_node_t> ast = parse_statement(tokens);

  std::map<std::string, std::string> symbol_table;
  gen_code_for_ast(ast, output_asm, symbol_table);

  system("fasm ../output/output.asm ../output/output.o");
  system("ld -o ../output/output ../output/output.o");

  info_msg("Outputted binary is found in output/output.asm");
  info_msg("Error count: {}", std::to_string(get_error_count()));
  reset_error_count();


  return 0;
}
