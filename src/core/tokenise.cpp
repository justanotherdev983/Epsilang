#include <string>
#include <stdexcept>

#include "core/tokenise.hpp"

#include <iostream>
#include <ostream>

#include "utils/error.hpp"


char consume(const std::string &contents, size_t &token_index) {
    return contents[token_index++];
}

char peek(const std::string &contents, size_t token_index) {
    if (token_index >= contents.length()) {
        return '\0';
    }
    return contents[token_index];
}

char peek_ahead(const std::string &contents, size_t token_index, size_t amount_ahead) {
    if (token_index + amount_ahead >= contents.length()) {
        return '\0';
    }
    return contents[token_index + amount_ahead];
}

std::vector<token_t> tokenise(const std::string &contents) {
    std::vector<token_t> tokens;
    size_t token_index = 0;

    while (peek(contents, token_index) != '\0') {
        token_t curr_token;

        if (isdigit(peek(contents, token_index))) {
            curr_token.type = token_type_e::type_int_lit;
            std::string num;
            while (isdigit(peek(contents, token_index))) {
                num += peek(contents, token_index);
                consume(contents, token_index);
            }
            curr_token.value = num;
        }
        else if (isalpha(peek(contents, token_index))) {
            std::string word;
            while (isalpha(peek(contents, token_index))) {
                word += peek(contents, token_index);
                consume(contents, token_index);
            }
            if (word == "exit")
            {
                curr_token.type = token_type_e::type_exit;
            }
            else
            {
                throw std::runtime_error("Invalid keyword");
            }
        }
        else if (peek(contents, token_index) == '*') {
            curr_token.type = token_type_e::type_mul;
            curr_token.value = std::string(1, consume(contents, token_index));
        }
        else if (peek(contents, token_index) == '/') {
            curr_token.type = token_type_e::type_div;
            curr_token.value = std::string(1, consume(contents, token_index));
        }
        else if (peek(contents, token_index) == '+') {
            curr_token.type = token_type_e::type_add;
            curr_token.value = std::string(1, consume(contents, token_index));
        }
        else if (peek(contents, token_index) == '-') {
            curr_token.type = token_type_e::type_sub;
            curr_token.value = std::string(1, consume(contents, token_index));
        }
        else if (peek(contents, token_index) == ';') {
            curr_token.type = token_type_e::type_semi;
            curr_token.value = std::string(1, consume(contents, token_index));
        }
        else if (isspace(peek(contents, token_index))) {
            while (isspace(peek(contents, token_index))) {
                curr_token.type = token_type_e::type_space;
                consume(contents, token_index);
            }
        }
        else {
            throw std::runtime_error("Invalid token");
        }

        tokens.push_back(curr_token);
    }

    // Add EOF token
    token_t eof_token;
    eof_token.type = token_type_e::type_EOF;
    tokens.push_back(eof_token);

    return tokens;
}