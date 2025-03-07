#pragma once

#include "core/parse.hpp"

void gen_binary_op(const ast_node_t &node, std::ofstream &asm_file);
void gen_code_for_ast(const std::vector<ast_node_t>& ast, std::ofstream &asm_file);
void gen_node_code(const ast_node_t &node, std::ofstream &asm_file);


void gen_if_code(const ast_node_t& node, std::ofstream& asm_file);
void gen_block_code(const ast_node_t& node, std::ofstream& asm_file);
void gen_comparison(const ast_node_t& node, std::ofstream& asm_file, const std::string& label_true, const std::string& label_end);
void process_node_declarations(ast_node_t& node);

void process_variable_declarations(std::vector<ast_node_t>& ast);