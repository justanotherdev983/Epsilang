#pragma once

#include <string>
#include <vector>

enum class token_type_e
{
    type_exit,
    type_int_lit,
    type_mul,
    type_div,
    type_add,
    type_sub,
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