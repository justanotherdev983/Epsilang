#pragma once

#include <string>
#include <vector>

enum class token_type_e
{
    type_exit,
    type_let,
    type_identifier,
    type_assignment,
    type_int_lit,
    type_mul,
    type_div,
    type_add,
    type_sub,
    type_eq,
    type_nq,
    type_ge,
    type_le,
    type_gt,
    type_lt,
    type_open_paren,
    type_close_paren,
    type_open_squigly,
    type_close_squigly,
    type_while,
    type_if,
    type_else,
    type_return,
    type_fn,
    type_call,
    type_comma,
    type_block,
    type_semi,
    type_space,
    type_EOF,
};

struct token_t
{
    token_type_e type;
    std::string value;
    std::string identifier;
};

char consume(const std::string &contents, size_t &token_index);
char peek(const std::string &contents, size_t token_index);

std::vector<token_t> tokenise(const std::string &contents);
