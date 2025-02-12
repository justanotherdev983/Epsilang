#pragma once

#include <string>
#include <vector>

enum class token_type_e
{
    type_exit,
    type_let,
    type_variable_name,
    type_equal,
    type_int_lit,
    type_mul,
    type_div,
    type_add,
    type_sub,
    type_open_paren,
    type_close_paren,
    type_semi,
    type_space,
    type_EOF,
};

struct token_t
{
    token_type_e type;
    std::string value;
};

char consume(const std::string &contents, size_t &token_index);
char peek(const std::string &contents, size_t token_index);

std::vector<token_t> tokenise(const std::string &contents);