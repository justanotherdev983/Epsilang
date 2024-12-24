#pragma once

#include <vector>
#include <memory>


#include "core/tokenise.hpp"

enum class ast_stmt_type_e
{
    type_exit_stmt,
};

//ast_stmt_type_e type;

struct ast_node_t
{
    token_type_e value;
    int int_value;
    std::unique_ptr<ast_node_t> child_node_1;
    std::unique_ptr<ast_node_t> child_node_2;
};

const token_t* peek_token(const std::vector<token_t>& tokens, const size_t &index);
const token_t* consume_token(const std::vector<token_t>& tokens, size_t &index);

std::vector<ast_node_t> parse_statement(const std::vector<token_t> &token_stream);