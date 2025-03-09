#pragma once

#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "core/parse.hpp"

struct code_gen_ctx_t {
  std::ofstream& asm_file;
  std::map<std::string, std::string>& symbol_table;
  std::map<std::string, ast_node_t*>& function_table;
  int variable_count = 0;

  ast_node_t* current_function =
      nullptr;  // Currently processed function (nullptr for global scope)

  code_gen_ctx_t(std::ofstream& asmFile,
                 std::map<std::string, std::string>& symbolTable,
                 std::map<std::string, ast_node_t*>& functionTable);

  std::string generate_label(const std::string& base_name);
  void access_variable(const std::string& var_name);
};

void gen_binary_op(const ast_node_t& node, code_gen_ctx_t& ctx);
void gen_code_for_ast(const std::vector<ast_node_t>& ast,
                      std::ofstream& asm_file,
                      std::map<std::string, std::string>& symbol_table);
void gen_node_code(const ast_node_t& node, code_gen_ctx_t& ctx);

void gen_if_code(const ast_node_t& node, code_gen_ctx_t& ctx);
void gen_block_code(const ast_node_t& node, code_gen_ctx_t& ctx);
void gen_comparison(const ast_node_t& node,
                    code_gen_ctx_t& ctx,
                    const std::string& label_true,
                    const std::string& label_end);
void process_node_declarations(ast_node_t& node, code_gen_ctx_t& ctx);

void process_variable_declarations(std::vector<ast_node_t>& ast,
                                   code_gen_ctx_t& ctx);
void process_function_declarations(std::vector<ast_node_t>& ast, code_gen_ctx_t& ctx);