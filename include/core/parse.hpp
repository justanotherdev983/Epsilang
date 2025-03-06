#pragma once

#include <vector>
#include <memory>


#include "core/tokenise.hpp"

struct ast_node_t
{
    token_type_e type;
    int int_value;
    std::string string_value;
    std::unique_ptr<ast_node_t> child_node_1;
    std::unique_ptr<ast_node_t> child_node_2;
    std::unique_ptr<ast_node_t> child_node_3;
};

const token_t* peek_token(const std::vector<token_t>& tokens, const size_t &index);
const token_t* consume_token(const std::vector<token_t>& tokens, size_t &index);

void parse_factor(std::vector<token_t>& tokens, size_t& token_index, ast_node_t& root_node);
void parse_term(std::vector<token_t>& tokens, size_t& index, ast_node_t& root_node);
void parse_expression(std::vector<token_t>& tokens, size_t& token_index, ast_node_t& root_node);

void parse_exit_statement(std::vector<token_t>& tokens, size_t& token_index, ast_node_t& root_node);
void parse_let_statement(std::vector<token_t>& token_stream, size_t &token_index, ast_node_t& root_node);
void parse_if_statement(std::vector<token_t>& token_stream, size_t &token_index, ast_node_t& root_node);

std::vector<ast_node_t> parse_statement(std::vector<token_t>& token_stream);
