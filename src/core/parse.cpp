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

        if (token->type == token_type_e::type_exit)
        {
            ast_node_t root_node;
            root_node.type = token->type;
            consume_token(token_stream, token_index);

            if (peek_token(token_stream, token_index)->type == token_type_e::type_space)
                consume_token(token_stream, token_index);


            token = peek_token(token_stream, token_index);
            if (token && token->type == token_type_e::type_int_lit)
            {
                root_node.child_node_1 = std::make_unique<ast_node_t>();
                root_node.child_node_1->type = token->type;
                root_node.child_node_1->int_value = stoi(token->value);
                consume_token(token_stream, token_index);
            }

            program_ast.push_back(std::move(root_node));
        }
        else if (token->type == token_type_e::type_space)
        {
            consume_token(token_stream, token_index);
        }
        else if (token->type == token_type_e::type_semi)
        {
            std::cout << "Semicolon encountered; End of statement" << std::endl;
            consume_token(token_stream, token_index);
        }
        else if (token->type == token_type_e::type_EOF)
        {
            std::cout << "End of file" << std::endl;
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