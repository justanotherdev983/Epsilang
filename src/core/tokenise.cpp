#include <string>
#include <ostream>

#include "core/tokenise.hpp"
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
        // Skip any whitespace characters
        if (isspace(peek(contents, token_index))) {
            while (isspace(peek(contents, token_index))) {
                consume(contents, token_index);
            }
            continue;  // Do not create a token for whitespace.
        }

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
                curr_token.type = token_type_e::type_exit;
            else if (word == "let")
                curr_token.type = token_type_e::type_let;
            else if (word ==  "if") {
                curr_token.type = token_type_e::type_if;
            }
            else if (word == "else") {
                curr_token.type = token_type_e::type_else;
            } 
            else
                curr_token.type = token_type_e::type_identifier;
            curr_token.value = word;
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
        else if (peek(contents, token_index) == '(') {
            curr_token.type = token_type_e::type_open_paren;
            curr_token.value = std::string(1, consume(contents, token_index));
        }
        else if (peek(contents, token_index) == ')') {
            curr_token.type = token_type_e::type_close_paren;
            curr_token.value = std::string(1, consume(contents, token_index));
        }
        else if (peek(contents, token_index) == '=') {
            // Check for ==
            if (peek_ahead(contents, token_index, 1) == '=') {
                curr_token.type = token_type_e::type_eq;
                curr_token.value = "==";
                consume(contents, token_index); // Consume first '='
                consume(contents, token_index); // Consume second '='
            } else {
                curr_token.type = token_type_e::type_equal;
                curr_token.value = std::string(1, consume(contents, token_index));
            }
        }
        else if (peek(contents, token_index) == '!') {
            // Check for !=
            if (peek_ahead(contents, token_index, 1) == '=') {
                curr_token.type = token_type_e::type_nq;
                curr_token.value = "!=";
                consume(contents, token_index); // Consume '!'
                consume(contents, token_index); // Consume '='
            } else {
                error_msg("Invalid token: expected '=' after '!'");
                consume(contents, token_index); // Consume '!'
                continue;
            }
        }
         else if (peek(contents, token_index) == '>') {
            // Check for >=
            if (peek_ahead(contents, token_index, 1) == '=') {
                curr_token.type = token_type_e::type_ge;
                curr_token.value = ">=";
                consume(contents, token_index); // Consume '>'
                consume(contents, token_index); // Consume '='
            } else {
                curr_token.type = token_type_e::type_gt; // greater than
                curr_token.value = std::string(1, consume(contents, token_index));
            }
        }
         else if (peek(contents, token_index) == '<') {
            // Check for <=
            if (peek_ahead(contents, token_index, 1) == '=') {
                curr_token.type = token_type_e::type_le;
                curr_token.value = "<=";
                consume(contents, token_index); // Consume '<'
                consume(contents, token_index); // Consume '='
            } else {
                curr_token.type = token_type_e::type_lt; // less than
                curr_token.value = std::string(1, consume(contents, token_index));
            }
        
        }else if (peek(contents, token_index) == '{') {
            curr_token.type = token_type_e::type_open_squigly;
            curr_token.value = std::string(1, consume(contents, token_index));
        }else if (peek(contents, token_index) == '}') {
            curr_token.type = token_type_e::type_close_squigly;
            curr_token.value = std::string(1, consume(contents, token_index));
        }
        else {
            error_msg("Invalid token");
            consume(contents, token_index); // Consume unknown character
            continue;
        }

        tokens.push_back(curr_token);
    }

    // Append the EOF token.
    token_t eof_token;
    eof_token.type = token_type_e::type_EOF;
    tokens.push_back(eof_token);

    return tokens;
}
