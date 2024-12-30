#include <iostream>
#include <string>

#include "core/parse.hpp"

const token_t* peek_token(const std::vector<token_t>& tokens, const size_t &index) {
    if (index < tokens.size()) {
        return &tokens[index];
    }
    return nullptr;
}

// Consume the current token and move to the next one
const token_t* consume_token(const std::vector<token_t>& tokens, size_t &index) {
    if (index < tokens.size()) {
        return &tokens[index++];
    }
    return nullptr;
}

bool is_math_operator(const token_t& token)
{
    if (token.type == token_type_e::type_add ||
                          token.type == token_type_e::type_sub ||
                          token.type == token_type_e::type_mul ||
                          token.type == token_type_e::type_div)
    {
        return true;
    }

    return false;
}

std::string token_type_to_string(token_type_e type) {
    switch (type) {
    case token_type_e::type_exit: return "type_exit";
    case token_type_e::type_int_lit: return "type_int_lit";
    case token_type_e::type_semi: return "type_semi";
    case token_type_e::type_space: return "type_space";
    case token_type_e::type_EOF: return "type_EOF";
    default: return "unknown_token_type";
    }
}



// Parse factor (integers or parenthesized expressions)
void parse_factor(std::vector<token_t>& tokens, size_t& token_index, ast_node_t& root_node) {
    const token_t* token = peek_token(tokens, token_index);

    if (!token) {
        std::cerr << "[ERROR]: Unexpected end of tokens while parsing factor." << std::endl;
        return;
    }

    if (token->type == token_type_e::type_int_lit) {
        root_node.type = token->type;
        root_node.int_value = stoi(token->value);
        std::cout << "[INFO]: Parsed integer literal: " << root_node.int_value << std::endl;
        consume_token(tokens, token_index);
    }
    else if (token->type == token_type_e::type_open_paren) {
        consume_token(tokens, token_index);
        parse_expression(tokens, token_index, root_node);

        token = peek_token(tokens, token_index);
        if (!token || token->type != token_type_e::type_close_paren) {
            std::cerr << "[ERROR]: Expected ')' but not found." << std::endl;
            return;
        }
        consume_token(tokens, token_index);
    }
    else {
        std::cerr << "[ERROR]: Invalid factor, expected integer literal or '(' but found: "
                  << token_type_to_string(token->type) << std::endl;
    }
}

// Parse multiplication and division operations
void parse_term(std::vector<token_t>& tokens, size_t& token_index, ast_node_t& root_node) {
    parse_factor(tokens, token_index, root_node);

    while (true) {
        const token_t* token = peek_token(tokens, token_index);
        if (!token) break;

        if (token->type == token_type_e::type_mul || token->type == token_type_e::type_div) {
            ast_node_t operator_node;
            operator_node.type = token->type;
            consume_token(tokens, token_index);

            operator_node.child_node_1 = std::make_unique<ast_node_t>(std::move(root_node));
            operator_node.child_node_2 = std::make_unique<ast_node_t>();

            parse_factor(tokens, token_index, *operator_node.child_node_2);
            root_node = std::move(operator_node);
        } else {
            break;
        }
    }
}

// Parse addition and subtraction operations
void parse_expression(std::vector<token_t>& tokens, size_t& token_index, ast_node_t& root_node) {
    parse_term(tokens, token_index, root_node);

    while (true) {
        const token_t* token = peek_token(tokens, token_index);
        if (!token) break;

        if (token->type == token_type_e::type_add || token->type == token_type_e::type_sub) {
            ast_node_t operator_node;
            operator_node.type = token->type;
            consume_token(tokens, token_index);

            operator_node.child_node_1 = std::make_unique<ast_node_t>(std::move(root_node));
            operator_node.child_node_2 = std::make_unique<ast_node_t>();

            parse_term(tokens, token_index, *operator_node.child_node_2);
            root_node = std::move(operator_node);
        } else {
            break;
        }
    }
}

void parse_exit_statement(std::vector<token_t>& tokens, size_t& token_index, ast_node_t& root_node) {
    // Consume exit token
    consume_token(tokens, token_index);

    const token_t* token = peek_token(tokens, token_index);
    if (!token || token->type != token_type_e::type_open_paren) {
        std::cerr << "[ERROR]: Expected '(' after 'exit'" << std::endl;
        return;
    }
    consume_token(tokens, token_index);

    // Parse the expression inside exit()
    ast_node_t expr_node;
    parse_expression(tokens, token_index, expr_node);


    token = peek_token(tokens, token_index);
    if (!token || token->type != token_type_e::type_close_paren) {
        std::cerr << "[ERROR]: Expected ')' after expression in exit statement" << std::endl;
        return;
    }
    consume_token(tokens, token_index);

    // Check for semicolon
    token = peek_token(tokens, token_index);
    if (!token || token->type != token_type_e::type_semi) {
        std::cerr << "[ERROR]: Expected ';' after exit statement" << std::endl;
        return;
    }
    consume_token(tokens, token_index);

    // Create exit node with expression as child
    root_node.type = token_type_e::type_exit;
    root_node.child_node_1 = std::make_unique<ast_node_t>(std::move(expr_node));
}

// Parse program statements
std::vector<ast_node_t> parse_statement(std::vector<token_t>& token_stream) {
    std::vector<ast_node_t> program_ast;
    size_t token_index = 0;

    while (token_index < token_stream.size()) {
        const token_t* token = peek_token(token_stream, token_index);
        if (!token) break;

        // Skip whitespace
        while (token && token->type == token_type_e::type_space) {
            consume_token(token_stream, token_index);
            token = peek_token(token_stream, token_index);
        }

        if (!token) break;

        if (token->type == token_type_e::type_exit) {
            ast_node_t root_node;
            parse_exit_statement(token_stream, token_index, root_node);
            program_ast.push_back(std::move(root_node));
        }
        else if (token->type == token_type_e::type_int_lit ||
                 token->type == token_type_e::type_open_paren) {
            ast_node_t root_node;
            parse_expression(token_stream, token_index, root_node);

            // Look for semicolon
            token = peek_token(token_stream, token_index);
            if (token && token->type == token_type_e::type_semi) {
                consume_token(token_stream, token_index);
            } else {
                std::cerr << "[ERROR]: Expected semicolon after expression" << std::endl;
            }

            program_ast.push_back(std::move(root_node));
        }
        else if (token->type == token_type_e::type_EOF) {
            break;
        }
        else {
            std::cerr << "[ERROR]: Unexpected token type: " << token_type_to_string(token->type) << std::endl;
            consume_token(token_stream, token_index);
        }
    }

    return program_ast;
}