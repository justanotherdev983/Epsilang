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

std::vector<ast_node_t> parse_statement(const std::vector<token_t> &token_stream)
{
    std::vector<ast_node_t> program_ast;
    size_t token_index = 0;

    while (token_index < token_stream.size())
    {
        const token_t* token = peek_token(token_stream, token_index);
        if (!token) break;

        if (token->type == token_type_e::type_exit) {
            ast_node_t root_node;
            root_node.type = token->type;
            consume_token(token_stream, token_index);

            // Handle optional space
            if (peek_token(token_stream, token_index)->type == token_type_e::type_space)
                consume_token(token_stream, token_index);

            // Ensure statement ends with a semicolon
            token = peek_token(token_stream, token_index);
            if (token && token->type == token_type_e::type_semi) {
                consume_token(token_stream, token_index);
            } else {
                std::cerr << "Error: Expected ';' after 'exit' statement!" << std::endl;
            }

            program_ast.push_back(std::move(root_node));
        }

        else if (token->type == token_type_e::type_int_lit) {
            ast_node_t root_node;

            root_node.child_node_1 = std::make_unique<ast_node_t>();
            root_node.child_node_1->type = token->type;
            std::cout << "Making int value first node" << stoi(token->value) << std::endl;
            root_node.child_node_1->int_value = stoi(token->value);
            consume_token(token_stream, token_index);

            // Parse operator
            token = peek_token(token_stream, token_index);
            if (token && (token->type == token_type_e::type_add ||
                          token->type == token_type_e::type_sub ||
                          token->type == token_type_e::type_mul ||
                          token->type == token_type_e::type_div))
                {
                    root_node.type = token->type;  // Set root type to operator
                    consume_token(token_stream, token_index);

                    // Parse second operand
                    token = peek_token(token_stream, token_index);
                    if (token && token->type == token_type_e::type_int_lit) {
                        root_node.child_node_2 = std::make_unique<ast_node_t>();
                        root_node.child_node_2->type = token->type;
                        std::cout << "Making int value for second node" << stoi(token->value) << std::endl;
                        root_node.child_node_2->int_value = stoi(token->value);
                        consume_token(token_stream, token_index);
                    }
                    else {
                        std::cerr << "Error: Expected integer literal after operator!" << std::endl;
                    }
                }

            // Expect semicolon to end expression
            token = peek_token(token_stream, token_index);
            if (token && token->type == token_type_e::type_semi) {
                consume_token(token_stream, token_index);
            } else {
                std::cerr << "Error: Expected ';' after expression!" << std::endl;
            }

            program_ast.push_back(std::move(root_node));
        }

        else if (token->type == token_type_e::type_space)
        {
            consume_token(token_stream, token_index);
        }
        else if (token->type == token_type_e::type_semi)
        {
            consume_token(token_stream, token_index);
        }
        else if (token->type == token_type_e::type_EOF)
        {
            consume_token(token_stream, token_index);
            break;
        }
        else
        {
            std::cout << "Unexpected token type: " << token_type_to_string(token->type) << std::endl;
            consume_token(token_stream, token_index);
        }
    }

    return program_ast;
}