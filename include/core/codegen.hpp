#pragma once

#include "core/parse.hpp"

void gen_binary_op(const ast_node_t &node, std::ofstream &asm_file);
void gen_code_for_ast(const std::vector<ast_node_t>& ast, std::ofstream &asm_file);
void gen_node_code(const ast_node_t &node, std::ofstream &asm_file);

void process_variable_declarations(std::vector<ast_node_t>& ast);