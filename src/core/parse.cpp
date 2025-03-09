#include <iostream>
#include <string>

#include "core/parse.hpp"
#include "core/tokenise.hpp"
#include "utils/error.hpp"

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

bool is_math_operator(const token_t& token) {
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
    case token_type_e::type_let: return "type_let";
    case token_type_e::type_identifier: return "type_identifier";
    case token_type_e::type_equal: return "type_equal";
    case token_type_e::type_int_lit: return "type_int_lit";
    case token_type_e::type_semi: return "type_semi";
    case token_type_e::type_space: return "type_space";
    case token_type_e::type_EOF: return "type_EOF";
    case token_type_e::type_if: return "type_if";
    case token_type_e::type_else: return "type_else";
    case token_type_e::type_eq: return "type_eq";
    case token_type_e::type_nq: return "type_nq";
    case token_type_e::type_ge: return "type_ge";
    case token_type_e::type_le: return "type_le";
    case token_type_e::type_lt: return "type_lt";
    case token_type_e::type_gt: return "type_gt";
    case token_type_e::type_block: return "type_block";
    case token_type_e::type_open_squigly: return "type_open_squigly";
    case token_type_e::type_close_squigly: return "type_close_squigly";
    case token_type_e::type_open_paren: return "type_open_paren";
    case token_type_e::type_close_paren: return "type_close_paren";
    case token_type_e::type_return: return "type_return";
    case token_type_e::type_fn: return "type_fn";
    case token_type_e::type_comma: return "type_comma";
    case token_type_e::type_call: return "type_call";
    case token_type_e::type_add: return "type_add";
    case token_type_e::type_sub: return "type_sub";
    case token_type_e::type_mul: return "type_mul";
    case token_type_e::type_div: return "type_div";
    default: return "unknown_token_type";
    }
}



// Parse factor (integers or parenthesized expressions)
void parse_factor(std::vector<token_t>& tokens, size_t& token_index,
                  ast_node_t& root_node) {
    const token_t* token = peek_token(tokens, token_index);

    if (!token || token->type == token_type_e::type_EOF) {
        error_msg("Unexpected end of tokens while parsing factor.");
        return;
    }

    if (token->type == token_type_e::type_int_lit) {
        root_node.type = token->type;
        root_node.int_value = stoi(token->value);
        info_msg("Parsed integer literal: {}", root_node.int_value);
        consume_token(tokens, token_index);
    } else if (token->type == token_type_e::type_identifier) {
        // Save the identifier value
        std::string identifier_name = token->value;
        consume_token(tokens, token_index);
        
        // Check if this is a function call
        token = peek_token(tokens, token_index);
        if (token && token->type == token_type_e::type_open_paren) {
            // This is a function call
            root_node.type = token_type_e::type_call;
            root_node.string_value = identifier_name; // Function name
            consume_token(tokens, token_index); // Consume '('
            
            // Parse arguments
            std::vector<ast_node_t> args;
            bool first_arg = true;
            
            while (true) {
                token = peek_token(tokens, token_index);
                if (!token) {
                    error_msg("Unexpected end of file in function arguments");
                    return;
                }
                
                if (token->type == token_type_e::type_close_paren) {
                    consume_token(tokens, token_index);
                    break;
                }
                
                // Handle comma between arguments
                if (!first_arg) {
                    if (token->type != token_type_e::type_comma) {
                        error_msg("Expected ',' between arguments, but found: {}", 
                                 token_type_to_string(token->type));
                        return;
                    }
                    consume_token(tokens, token_index);
                }
                
                // Parse argument expression
                ast_node_t arg;
                parse_expression(tokens, token_index, arg);
                args.push_back(std::move(arg));
                first_arg = false;
            }
            
            // Store arguments in the function call node
            root_node.arguments = std::move(args);
        } else {
            // This is a variable reference
            root_node.type = token_type_e::type_identifier;
            root_node.string_value = identifier_name;
            info_msg("Parsed identifier: {}", root_node.string_value);
        }
    } else if (token->type == token_type_e::type_open_paren) {
        consume_token(tokens, token_index);
        parse_expression(tokens, token_index, root_node);

        token = peek_token(tokens, token_index);
        if (!token || token->type != token_type_e::type_close_paren) {
            error_msg("Expected ')', but found: {}", 
                     token ? token_type_to_string(token->type) : "EOF");
            return;
        }
        consume_token(tokens, token_index);
    } else {
        error_msg(
            "Invalid factor, expected integer literal or '(' but found: {}",
            token_type_to_string(token->type));
    }
}

void parse_return_statement(std::vector<token_t>& tokens, size_t& token_index, ast_node_t& root_node) {
    // Consume 'return' token
    consume_token(tokens, token_index);
    
    root_node.type = token_type_e::type_return;
    
    // Parse the return expression
    ast_node_t expr_node;
    parse_expression(tokens, token_index, expr_node);
    root_node.child_node_1 = std::make_unique<ast_node_t>(std::move(expr_node));
    
    const token_t* token = peek_token(tokens, token_index);
    if (!token || token->type != token_type_e::type_semi) {
        error_msg("Expected ';' after return expression, but found: {}", 
                 token_type_to_string(token->type));
        return;
    }
    consume_token(tokens, token_index);
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

void parse_function_statement(std::vector<token_t>& tokens, size_t& token_index, ast_node_t& root_node) {
    consume_token(tokens, token_index); // Consume 'fn' token
    
    const token_t* func_name_token = peek_token(tokens, token_index);
    if (!func_name_token || func_name_token->type != token_type_e::type_identifier) {
        error_msg("Expected function name but found: {}", 
                 func_name_token ? token_type_to_string(func_name_token->type) : "EOF");
        return;
    }
    
    root_node.type = token_type_e::type_fn;
    root_node.string_value = func_name_token->value;
    consume_token(tokens, token_index);

    const token_t* open_paren_token = peek_token(tokens, token_index);
    if (!open_paren_token || open_paren_token->type != token_type_e::type_open_paren) {
        error_msg("Expected '(' but found: {}", 
                 open_paren_token ? token_type_to_string(open_paren_token->type) : "EOF");
        return;
    }
    consume_token(tokens, token_index);

    std::vector<std::string> parameters;
    bool first_parameter = true;

    while (true) {
        const token_t* token = peek_token(tokens, token_index);
        if (!token) {
            error_msg("Unexpected end of file in function parameters");
            return;
        }

        if (token->type == token_type_e::type_close_paren) {
            consume_token(tokens, token_index);
            break;
        }

        if (!first_parameter) {
            if (token->type != token_type_e::type_comma) {
                error_msg("Expected ',' between function args but found: {}", 
                         token_type_to_string(token->type));
                return;
            }
            consume_token(tokens, token_index);
            token = peek_token(tokens, token_index);
            if (!token) {
                error_msg("Unexpected end of file after comma in function parameters");
                return;
            }
        }
        
        if (token->type != token_type_e::type_identifier) {
            error_msg("Expected parameter name but found: {}", 
                     token_type_to_string(token->type));
            return;
        }
        parameters.push_back(token->value);
        consume_token(tokens, token_index);
        first_parameter = false;
    }

    root_node.parameters = std::move(parameters);

    const token_t* squigly_token = peek_token(tokens, token_index);
    if (!squigly_token || squigly_token->type != token_type_e::type_open_squigly) {
        error_msg("Expected '{' but found: {}", 
                 squigly_token ? token_type_to_string(squigly_token->type) : "EOF");
        return;
    }
    consume_token(tokens, token_index);

    std::vector<ast_node_t> body_statements;
    
    while (true) {
        const token_t* token = peek_token(tokens, token_index);
        if (!token) {
            error_msg("Unexpected end of file in function body");
            return;
        }
        
        if (token->type == token_type_e::type_close_squigly) {
            consume_token(tokens, token_index);
            break;
        }
        
        ast_node_t statement;
        if (token->type == token_type_e::type_let) {
            parse_let_statement(tokens, token_index, statement);
        } 
        else if (token->type == token_type_e::type_if) {
            parse_if_statement(tokens, token_index, statement);
        }
        else if (token->type == token_type_e::type_exit) {
            parse_exit_statement(tokens, token_index, statement);
        }
        else if (token->type == token_type_e::type_return) {
            parse_return_statement(tokens, token_index, statement);
        }
        else if (token->type == token_type_e::type_int_lit || 
                 token->type == token_type_e::type_identifier || 
                 token->type == token_type_e::type_open_paren) {
            parse_expression(tokens, token_index, statement);
            
            token = peek_token(tokens, token_index);
            if (token && token->type == token_type_e::type_semi) {
                consume_token(tokens, token_index);
            } else {
                error_msg("Expected ';' after expression in function body, but found: {}", 
                         token ? token_type_to_string(token->type) : "EOF");
                // Attempt to recover from error by skipping to next semicolon
                while (token && token->type != token_type_e::type_semi && 
                       token->type != token_type_e::type_close_squigly) {
                    consume_token(tokens, token_index);
                    token = peek_token(tokens, token_index);
                }
                if (token && token->type == token_type_e::type_semi) {
                    consume_token(tokens, token_index);
                }
                continue;
            }
        }
        else {
            error_msg("Unexpected token in function body: {}", token_type_to_string(token->type));
            // Attempt to recover from error by skipping to next semicolon or closing brace
            while (token && token->type != token_type_e::type_semi && 
                   token->type != token_type_e::type_close_squigly) {
                consume_token(tokens, token_index);
                token = peek_token(tokens, token_index);
            }
            if (token && token->type == token_type_e::type_semi) {
                consume_token(tokens, token_index);
            }
            continue;
        }
        
        body_statements.push_back(std::move(statement));
    }

    root_node.body = std::move(body_statements);
}

void parse_exit_statement(std::vector<token_t>& tokens, size_t& token_index, ast_node_t& root_node) {
    // Consume exit token
    consume_token(tokens, token_index);

    const token_t* token = peek_token(tokens, token_index);
    if (!token || token->type != token_type_e::type_open_paren) {
        error_msg("Expected '(' but found: {}", token_type_to_string(token->type));
        return;
    }
    consume_token(tokens, token_index);

    // Parse the expression inside exit()
    ast_node_t expr_node;
    parse_expression(tokens, token_index, expr_node);


    token = peek_token(tokens, token_index);
    if (!token || token->type != token_type_e::type_close_paren) {
        error_msg("Expected ')' in exit statement, but found: {}", token_type_to_string(token->type));
        return;
    }
    consume_token(tokens, token_index);

    // Check for semicolon
    token = peek_token(tokens, token_index);
    if (!token || token->type != token_type_e::type_semi) {
        error_msg("Expected ';' after exit statement, but found: {}", token_type_to_string(token->type));
        return;
    }
    consume_token(tokens, token_index);

    // Create let node with expression as child
    root_node.type = token_type_e::type_exit;
    root_node.child_node_1 = std::make_unique<ast_node_t>(std::move(expr_node));
}

void parse_let_statement(std::vector<token_t>& tokens, size_t &token_index, ast_node_t& root_node) {
    consume_token(tokens, token_index); // Let token

    const token_t* id_token = peek_token(tokens, token_index);
    if (!id_token || id_token->type != token_type_e::type_identifier) {
        error_msg("Expected variable name but found: {}", token_type_to_string(id_token->type));
        return;
    }

    // Create an identifier node
    ast_node_t identifier_node;
    identifier_node.type = token_type_e::type_identifier;
    identifier_node.string_value = id_token->value; // Store the identifier name
    consume_token(tokens, token_index); // Consume the identifier token

    const token_t* equal_token = peek_token(tokens, token_index);
    if (!equal_token || equal_token->type != token_type_e::type_equal) {
        error_msg("Expected '=' in let statement, but found: {}", token_type_to_string(equal_token->type));
        return;
    }
    consume_token(tokens, token_index); // Consume the '=' token

    // Parse the expression
    ast_node_t expr_node;
    parse_expression(tokens, token_index, expr_node);

    // Create an assignment node
    root_node.type = token_type_e::type_let; 
    root_node.child_node_1 = std::make_unique<ast_node_t>(std::move(identifier_node)); // Left child is the identifier
    root_node.child_node_2 = std::make_unique<ast_node_t>(std::move(expr_node));       // Right child is the expression

    // Check for semicolon
    const token_t* semi_token = peek_token(tokens, token_index);
    if (!semi_token || semi_token->type != token_type_e::type_semi) {
        error_msg("Expected ';' after let statement, but found: {}", token_type_to_string(semi_token->type));
        return;
    }
    consume_token(tokens, token_index); // Consume the ';' token
}

void parse_comparison(std::vector<token_t>& tokens, size_t& token_index, ast_node_t& root_node) {
    parse_expression(tokens, token_index, root_node);

    const token_t* token = peek_token(tokens, token_index);
    if (!token) return;

    if (token->type == token_type_e::type_eq || 
        token->type == token_type_e::type_nq || 
        token->type == token_type_e::type_ge || 
        token->type == token_type_e::type_le) {
        
        ast_node_t operator_node;
        operator_node.type = token->type;
        consume_token(tokens, token_index);

        operator_node.child_node_1 = std::make_unique<ast_node_t>(std::move(root_node));
        operator_node.child_node_2 = std::make_unique<ast_node_t>();

        parse_expression(tokens, token_index, *operator_node.child_node_2);
        root_node = std::move(operator_node);
    }
}

void parse_if_statement(std::vector<token_t>& tokens, size_t &token_index, ast_node_t& root_node) {
    consume_token(tokens, token_index); // if token
    
    const token_t* open_paren = peek_token(tokens, token_index);
    if (!open_paren || open_paren->type != token_type_e::type_open_paren) {
        error_msg("Expected '(' after if statement, but found: {}", 
                 open_paren ? token_type_to_string(open_paren->type) : "EOF");
        return;
    }
    consume_token(tokens, token_index); // Consume '('
    
    // Parse condition expression
    ast_node_t condition_node;
    parse_comparison(tokens, token_index, condition_node);
    
    // Check for closing parenthesis
    const token_t* close_paren = peek_token(tokens, token_index);
    if (!close_paren || close_paren->type != token_type_e::type_close_paren) {
        error_msg("Expected ')' after if condition, but found: {}", 
                 close_paren ? token_type_to_string(close_paren->type) : "EOF");
        return;
    }
    consume_token(tokens, token_index); // Consume ')'
    
    // Check for opening brace
    const token_t* open_squigly = peek_token(tokens, token_index);
    if (!open_squigly || open_squigly->type != token_type_e::type_open_squigly) {
        error_msg("Expected '{' after if condition, but found: {}", 
                 open_squigly ? token_type_to_string(open_squigly->type) : "EOF");
        return;
    }
    consume_token(tokens, token_index); // Consume '{'
    
    // Parse the then branch (statements inside the if block)
    ast_node_t then_branch;
    then_branch.type = token_type_e::type_block;
    std::vector<ast_node_t> then_statements;
    
    // Parse statements until we hit the closing brace
    while (true) {
        const token_t* token = peek_token(tokens, token_index);
        if (!token) {
            error_msg("Unexpected end of file in if block");
            break;
        }
        
        if (token->type == token_type_e::type_close_squigly) {
            consume_token(tokens, token_index); // Consume '}'
            break;
        }
        
        // Parse a statement and add it to the block
        ast_node_t statement;
        if (token->type == token_type_e::type_let) {
            parse_let_statement(tokens, token_index, statement);
        } 
        else if (token->type == token_type_e::type_if) {
            parse_if_statement(tokens, token_index, statement);
        }
        else if (token->type == token_type_e::type_exit) {
            parse_exit_statement(tokens, token_index, statement);
        }
        else if (token->type == token_type_e::type_return) {
            parse_return_statement(tokens, token_index, statement);
        }
        else if (token->type == token_type_e::type_int_lit || 
                 token->type == token_type_e::type_identifier || 
                 token->type == token_type_e::type_open_paren) {
            parse_expression(tokens, token_index, statement);
            
            // Look for semicolon
            token = peek_token(tokens, token_index);
            if (token && token->type == token_type_e::type_semi) {
                consume_token(tokens, token_index);
            } else {
                error_msg("Expected ';' after expression in if block, but found: {}", 
                         token ? token_type_to_string(token->type) : "EOF");
                // Try to recover by skipping to next semicolon or closing brace
                while (token && token->type != token_type_e::type_semi && 
                       token->type != token_type_e::type_close_squigly) {
                    consume_token(tokens, token_index);
                    token = peek_token(tokens, token_index);
                }
                if (token && token->type == token_type_e::type_semi) {
                    consume_token(tokens, token_index);
                }
                continue;
            }
        }
        else {
            error_msg("Unexpected token in if block: {}", token_type_to_string(token->type));
            // Skip to next statement
            while (token && token->type != token_type_e::type_semi && 
                   token->type != token_type_e::type_close_squigly) {
                consume_token(tokens, token_index);
                token = peek_token(tokens, token_index);
            }
            if (token && token->type == token_type_e::type_semi) {
                consume_token(tokens, token_index);
            }
            continue;
        }
        
        then_statements.push_back(std::move(statement));
    }
    
    // Store the statements in the then branch
    then_branch.statements = std::move(then_statements);
    
    // Check for else branch
    const token_t* else_token = peek_token(tokens, token_index);
    ast_node_t else_branch;
    bool has_else = false;
    
    if (else_token && else_token->type == token_type_e::type_else) {
        consume_token(tokens, token_index); // Consume 'else'
        has_else = true;
        
        // Check if it's an else-if
        const token_t* next_token = peek_token(tokens, token_index);
        if (next_token && next_token->type == token_type_e::type_if) {
            // Parse the else-if as a nested if statement
            parse_if_statement(tokens, token_index, else_branch);
        } else {
            // Parse the else block
            const token_t* else_open_squigly = peek_token(tokens, token_index);
            if (!else_open_squigly || else_open_squigly->type != token_type_e::type_open_squigly) {
                error_msg("Expected '{' after else, but found: {}", 
                         else_open_squigly ? token_type_to_string(else_open_squigly->type) : "EOF");
                return;
            }
            consume_token(tokens, token_index); // Consume '{'
            
            else_branch.type = token_type_e::type_block;
            std::vector<ast_node_t> else_statements;
            
            // Parse statements until we hit the closing brace
            while (true) {
                const token_t* token = peek_token(tokens, token_index);
                if (!token) {
                    error_msg("Unexpected end of file in else block");
                    break;
                }
                
                if (token->type == token_type_e::type_close_squigly) {
                    consume_token(tokens, token_index); // Consume '}'
                    break;
                }
                
                // Parse a statement and add it to the block
                ast_node_t statement;
                if (token->type == token_type_e::type_let) {
                    parse_let_statement(tokens, token_index, statement);
                } 
                else if (token->type == token_type_e::type_if) {
                    parse_if_statement(tokens, token_index, statement);
                }
                else if (token->type == token_type_e::type_exit) {
                    parse_exit_statement(tokens, token_index, statement);
                }
                else if (token->type == token_type_e::type_return) {
                    parse_return_statement(tokens, token_index, statement);
                }
                else if (token->type == token_type_e::type_int_lit || 
                         token->type == token_type_e::type_identifier || 
                         token->type == token_type_e::type_open_paren) {
                    parse_expression(tokens, token_index, statement);
                    
                    // Look for semicolon
                    token = peek_token(tokens, token_index);
                    if (token && token->type == token_type_e::type_semi) {
                        consume_token(tokens, token_index);
                    } else {
                        error_msg("Expected ';' after expression in else block, but found: {}", 
                                 token ? token_type_to_string(token->type) : "EOF");
                        // Try to recover
                        while (token && token->type != token_type_e::type_semi && 
                               token->type != token_type_e::type_close_squigly) {
                            consume_token(tokens, token_index);
                            token = peek_token(tokens, token_index);
                        }
                        if (token && token->type == token_type_e::type_semi) {
                            consume_token(tokens, token_index);
                        }
                        continue;
                    }
                }
                else {
                    error_msg("Unexpected token in else block: {}", token_type_to_string(token->type));
                    // Skip to next statement
                    while (token && token->type != token_type_e::type_semi && 
                           token->type != token_type_e::type_close_squigly) {
                        consume_token(tokens, token_index);
                        token = peek_token(tokens, token_index);
                    }
                    if (token && token->type == token_type_e::type_semi) {
                        consume_token(tokens, token_index);
                    }
                    continue;
                }
                
                else_statements.push_back(std::move(statement));
            }
            
            // Store the statements in the else branch
            else_branch.statements = std::move(else_statements);
        }
    }
    
    // Create the if node with condition, then branch, and optional else branch
    root_node.type = token_type_e::type_if;
    root_node.child_node_1 = std::make_unique<ast_node_t>(std::move(condition_node)); // Condition
    root_node.child_node_2 = std::make_unique<ast_node_t>(std::move(then_branch));    // Then branch
    
    if (has_else) {
        root_node.child_node_3 = std::make_unique<ast_node_t>(std::move(else_branch)); // Else branch
    }
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
                error_msg("Expected ';' after expression, but found: {}", token_type_to_string(token->type));
            }

            program_ast.push_back(std::move(root_node));
        } else if(token->type == token_type_e::type_let) {
            ast_node_t root_node;
            parse_let_statement(token_stream, token_index, root_node);
            program_ast.push_back(std::move(root_node));
        } else if (token->type == token_type_e::type_if) {
            ast_node_t root_node;
            parse_if_statement(token_stream, token_index, root_node);
            program_ast.push_back(std::move(root_node));
        } else if (token->type == token_type_e::type_fn) {
            ast_node_t root_node;
            parse_function_statement(token_stream, token_index, root_node);
            program_ast.push_back(std::move(root_node));
        }
        else if (token->type == token_type_e::type_EOF) {
            break;
        }
        else {
            error_msg("Unexpected token type: {}", token_type_to_string(token->type));
            consume_token(token_stream, token_index);
        }
    }

    return program_ast;
}
